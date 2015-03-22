#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

#ifndef user_life_hpp
#define user_life_hpp

#include "puzzler/puzzles/life.hpp"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"

// To go at the top of the file - OpenCL
#include <fstream>
#include <streambuf>

class LifeProvider
	: public puzzler::LifePuzzle
{

private:
	bool update_unroll(int n, const std::vector<bool> &curr, int x, int y) const
	{
		int neighbours = 0;
		int ox;
		int oy;
		//dx = -1, dy = -1

		ox = (n + x - 1) % n; // handle wrap-around
		oy = (n + y - 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = -1, dy = 0
		ox = (n + x - 1) % n; // handle wrap-around
		oy = (n + y) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = -1, dy = 1
		ox = (n + x - 1) % n; // handle wrap-around
		oy = (n + y + 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = 0, dy = -1
		ox = (n + x) % n; // handle wrap-around
		oy = (n + y - 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = 0, dy = 0
		ox = (n + x) % n; // handle wrap-around
		oy = (n + y) % n;

		//no if loop because !(dx == 0 && dy == 0) = 0

		//dx = 0, dy = 1
		ox = (n + x) % n; // handle wrap-around
		oy = (n + y + 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = 1, dy = -1

		ox = (n + x + 1) % n; // handle wrap-around
		oy = (n + y - 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = 1, dy = 0
		ox = (n + x + 1) % n; // handle wrap-around
		oy = (n + y) % n;

		if (curr.at(oy*n + ox))
			neighbours++;

		//dx = 1, dy = 1
		ox = (n + x + 1) % n; // handle wrap-around
		oy = (n + y + 1) % n;

		if (curr.at(oy*n + ox))
			neighbours++;


		if (curr[n*y + x]){
			// alive
			if (neighbours<2){
				return false;
			}
			else if (neighbours>3){
				return false;
			}
			else{
				return true;
			}
		}
		else{
			// dead
			if (neighbours == 3){
				return true;
			}
			else{
				return false;
			}
		}
	}
public:
	LifeProvider()
	{}

	virtual void Execute(
		puzzler::ILog *log,
		const puzzler::LifeInput *input,
		puzzler::LifeOutput *output
		) const override {
		/*ReferenceExecute(log, input, output);*/

		log->LogVerbose("About to start running iterations (total = %d)", input->steps);

		unsigned n = input->n;
		std::vector<bool> state = input->state;

		log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
			dst << "\n";
			for (unsigned y = 0; y < n; y++){
				for (unsigned x = 0; x < n; x++){
					dst << (state.at(y*n + x) ? 'x' : ' ');
				}
				dst << "\n";
			}
		});
		
		//Choosing TBB or OpenCL
		int opencl_flag = 0;
		if (getenv("HPCE_SELECT_OPENCL")){
			opencl_flag = atoi(getenv("HPCE_SELECT_OPENCL"));
		}

		if (opencl_flag == 1){

			//Initialise OpenCL
			std::vector<cl::Platform> platforms;

			cl::Platform::get(&platforms);
			if (platforms.size() == 0)
				throw std::runtime_error("No OpenCL platforms found.");
			
			std::cerr << "Found " << platforms.size() << " platforms\n";
			for (unsigned i = 0; i < platforms.size(); i++){
				std::string vendor = platforms[i].getInfo<CL_PLATFORM_VENDOR>();
				std::cerr << "  Platform " << i << " : " << vendor << "\n";
			}

			int selectedPlatform = 0;
			if (getenv("HPCE_SELECT_PLATFORM")){
				selectedPlatform = atoi(getenv("HPCE_SELECT_PLATFORM"));
			}
			std::cerr << "Choosing platform " << selectedPlatform << "\n";
			cl::Platform platform = platforms.at(selectedPlatform);

			std::vector<cl::Device> devices;
			platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
			if (devices.size() == 0){
				throw std::runtime_error("No opencl devices found.\n");
			}

			std::cerr << "Found " << devices.size() << " devices\n";
			for (unsigned i = 0; i < devices.size(); i++){
				std::string name = devices[i].getInfo<CL_DEVICE_NAME>();
				std::cerr << "  Device " << i << " : " << name << "\n";
			}

			int selectedDevice = 0;
			if (getenv("HPCE_SELECT_DEVICE")){
				selectedDevice = atoi(getenv("HPCE_SELECT_DEVICE"));
			}

			std::cerr << "Choosing device " << selectedDevice << "\n";
			cl::Device device = devices.at(selectedDevice);

			cl::Context context(devices);

			std::string kernelSource = LoadSource("user_life.cl");

			cl::Program::Sources sources;   // A vector of (data,length) pairs
			sources.push_back(std::make_pair(kernelSource.c_str(), kernelSource.size() + 1)); // push on our single string

			cl::Program program(context, sources);
			try{
				program.build(devices);
			}
			catch (...){
				for (unsigned i = 0; i < devices.size(); i++){
					std::cerr << "Log for device " << devices[i].getInfo<CL_DEVICE_NAME>() << ":\n\n";
					std::cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i]) << "\n\n";
				}
				throw;
			}

			size_t cbBuffer = 4 * n*n;
			cl::Buffer currbuf(context, CL_MEM_READ_WRITE, cbBuffer, NULL);
			cl::Buffer nextbuf(context, CL_MEM_READ_WRITE, cbBuffer, NULL);

			cl::CommandQueue queue(context, device);

			std::vector<int> state_int(state.begin(), state.end());

			queue.enqueueWriteBuffer(currbuf, CL_TRUE, 0, cbBuffer, &state_int[0]);

			cl::NDRange offset(0, 0);               // Always start iterations at x=0, y=0
			cl::NDRange globalSize(n, n);   // Global size must match the original loops
			cl::NDRange localSize = cl::NullRange;    // We don't care about local size

			cl::Kernel kernel_updateCL(program, "update_cl");



			for (unsigned i = 0; i < input->steps; i++){


				log->LogVerbose("OpenCL: Starting iteration %d of %d\n", i, input->steps);
				kernel_updateCL.setArg(0, currbuf);
				kernel_updateCL.setArg(1, nextbuf);
				queue.enqueueNDRangeKernel(kernel_updateCL, offset, globalSize, localSize);
				queue.enqueueBarrier();

				std::swap(currbuf, nextbuf);
				// The weird form of log is so that there is little overhead
				// if logging is disabled
				log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
					dst << "\n";
					for (unsigned y = 0; y < n; y++){
						for (unsigned x = 0; x < n; x++){
							dst << (state[y*n + x] ? 'x' : ' ');
						}
						dst << "\n";
					}
				});
			}
			log->LogVerbose("Finished steps");

			if (opencl_flag == 1) {
				queue.enqueueReadBuffer(currbuf, CL_TRUE, 0, cbBuffer, &state_int[0]);


				for (unsigned i = 0; i < n*n; i++){
					state[i] = (bool)state_int[i];
				}
			}

			output->state = state;
		}
		else{
			for (unsigned i = 0; i < input->steps; i++){
				log->LogVerbose("TBB: Starting iteration %d of %d\n", i, input->steps);

				std::vector<bool> next(n*n);
				//Parallelised next[]=
				unsigned K = 10;

				auto f = [&](const tbb::blocked_range2d<unsigned> &chunk) {
					for (unsigned x = chunk.rows().begin(); x != chunk.rows().end(); x++){
						for (unsigned y = chunk.cols().begin(); y != chunk.cols().end(); y++){
							next[y*n + x] = update_unroll(n, state, x, y);
						}
					}
				};
				tbb::parallel_for(tbb::blocked_range2d<unsigned>(0, n, K, 0, n, K), f, tbb::simple_partitioner());
				state = next;



				// The weird form of log is so that there is little overhead
				// if logging is disabled
				log->Log(puzzler::Log_Debug, [&](std::ostream &dst){
					dst << "\n";
					for (unsigned y = 0; y < n; y++){
						for (unsigned x = 0; x < n; x++){
							dst << (state[y*n + x] ? 'x' : ' ');
						}
						dst << "\n";
					}
				});
			}
			log->LogVerbose("Finished steps");
			output->state = state;
		}
	}


};

#endif
