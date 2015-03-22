#ifndef user_matrix_exponent_hpp
#define user_matrix_exponent_hpp

#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

#include "puzzler/puzzles/matrix_exponent.hpp"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"

// To go at the top of the file - OpenCL
#include <fstream>
#include <streambuf>

//OpenCL contents
std::string LoadSource(const char *fileName)
{
	// Don't forget to change your_login here
	std::string baseDir = "provider";
	if (getenv("HPCE_CL_SRC_DIR")){
		baseDir = getenv("HPCE_CL_SRC_DIR");
	}

	std::string fullName = baseDir + "/" + fileName;

	// Open a read-only binary stream over the file
	std::ifstream src(fullName, std::ios::in | std::ios::binary);
	if (!src.is_open())
		throw std::runtime_error("LoadSource : Couldn't load cl file from '" + fullName + "'.");

	// Read all characters of the file into a string
	return std::string(
		(std::istreambuf_iterator<char>(src)), // Node the extra brackets.
		std::istreambuf_iterator<char>()
		);
}

class MatrixExponentProvider
  : public puzzler::MatrixExponentPuzzle
{

private:
	static std::vector<uint32_t> MatrixMul_tbb(unsigned n, std::vector<uint32_t> a, std::vector<uint32_t> b)
	{
		std::vector<uint32_t> res(n*n, 0);

		unsigned K = 32;

		auto f = [&](const tbb::blocked_range2d<unsigned> &chunk) {
			for (unsigned r = chunk.rows().begin(); r !=chunk.rows().end(); r++){
				for (unsigned c = chunk.cols().begin(); c != chunk.cols().end(); c++){
					for (unsigned i = 0; i<n; i++){
						res[r*n + c] = Add(res[r*n + c], Mul(a[r*n + i], b[i*n + r]));
					}
				}
			}
		};
		tbb::parallel_for(tbb::blocked_range2d<unsigned>(0, n, K, 0, n, K), f , tbb::simple_partitioner() ); //chunking over 2D blocked range

		return res;
	}

public:
  MatrixExponentProvider()
  {}

  virtual void Execute(
		       puzzler::ILog *log,
		       const puzzler::MatrixExponentInput *input,
		       puzzler::MatrixExponentOutput *output
		       ) const override {
	  std::vector<uint32_t> hash(input->steps);

	  log->LogVerbose("Setting up A and identity");
	  auto A = MatrixCreate(input->n, input->seed);
	  auto acc = MatrixIdentity(input->n);

	  log->LogVerbose("Beginning multiplication");
	  hash[0] = acc[0];

	  //Choosing TBB or OpenCL
	  /*int opencl_flag = 0;
	  if (getenv("HPCE_SELECT_OPENCL")){
		  opencl_flag = atoi(getenv("HPCE_SELECT_OPENCL"));
	  }*/

	  if (input->n < 32) {
		  return ReferenceExecute(log, input, output);
	  }
	  else if (input->n <= 64) {
		  for (unsigned i = 1; i < input->steps; i++){
			  log->LogDebug("TBB: Iteration %d", i);
			  acc = MatrixMul_tbb(input->n, acc, A);
			  hash[i] = acc[0];
		  }
	  } else {

		  std::vector<cl::Platform> platforms;

		  cl::Platform::get(&platforms);
		  if (platforms.size() == 0)
			  throw std::runtime_error("No OpenCL platforms found.");

		  std::cerr << "Found " << platforms.size() << " platforms\n";
		  for (unsigned i = 0; i<platforms.size(); i++){
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
		  for (unsigned i = 0; i<devices.size(); i++){
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

		  std::string kernelSource = LoadSource("user_matrix_exponent.cl");

		  cl::Program::Sources sources;   // A vector of (data,length) pairs
		  sources.push_back(std::make_pair(kernelSource.c_str(), kernelSource.size() + 1)); // push on our single string

		  cl::Program program(context, sources);
		  try{
			  program.build(devices);
		  }
		  catch (...){
			  for (unsigned i = 0; i<devices.size(); i++){
				  std::cerr << "Log for device " << devices[i].getInfo<CL_DEVICE_NAME>() << ":\n\n";
				  std::cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[i]) << "\n\n";
			  }
			  throw;
		  }
		  
		  unsigned cbBuffer = 4*input->n*input->n;
		  std::vector<uint32_t> res(input->n*input->n, 0);

		  cl::Buffer accbuf(context, CL_MEM_READ_WRITE, cbBuffer);
		  cl::Buffer resbuf(context, CL_MEM_READ_WRITE, cbBuffer);
		  cl::Buffer Abuf(context, CL_MEM_READ_ONLY, cbBuffer);

		  cl::CommandQueue queue(context, device);
		  queue.enqueueWriteBuffer(accbuf, CL_TRUE, 0, cbBuffer, &acc[0]);
		  queue.enqueueWriteBuffer(resbuf, CL_TRUE, 0, cbBuffer, &res[0]);
		  queue.enqueueWriteBuffer(Abuf, CL_TRUE, 0, cbBuffer, &A[0]);
		  

		  cl::NDRange offset(0,0);               // Always start iterations at x=0, y=0
		  cl::NDRange globalSize(input->n,input->n);   // Global size must match the original loops
		  cl::NDRange localSize = cl::NullRange;    // We don't care about local size
	
		  cl::Kernel kernel_MatrixMul(program, "MatrixMul");


		  for (unsigned i = 1; i<input->steps; i++){
			  log->LogDebug("OpenCL: Iteration %d", i);

			  kernel_MatrixMul.setArg(0, accbuf);			 
			  kernel_MatrixMul.setArg(1, Abuf);			
			  kernel_MatrixMul.setArg(2, resbuf);
			  queue.enqueueNDRangeKernel(kernel_MatrixMul, offset, globalSize, localSize);
			  queue.enqueueBarrier();
			  std::swap(accbuf, resbuf);
			  
			  queue.enqueueReadBuffer(accbuf, CL_TRUE, 0, 4, &hash[i]); //Write the required bytes into hash[i]
		  }
	  }

	  log->LogVerbose("Done");

	  output->hashes = hash;
  }

};

#endif
