#include <iostream>
#include <fstream>
#include <cassert>
#include <iterator>
#include <set>
#include <cstring>

typedef unsigned int uint;
typedef unsigned short int usint;
typedef unsigned char byte;

#define IF 0
#define ID 1
#define EX 2
#define MM 3
#define WB 4


int log2(uint x)
{
    int r = 0;
    while(x > 0)
    {
        x = x >> 1;
        r++;
    }

    return r - 1;
}

int pow2(uint n)
{
    int r = 1;
    while(n > 0)
    {
        r = r << 1;
        n--;
    }

    return r;
}
class Cache
{
	uint cacheSize = 256;
	uint blockSize = 4;
	std::fstream srcFile;
	std::vector<uint> sets;
	int num_of_reads = 0;
	int num_of_writes = 0;
	//Little Endian
public:
	Cache(std::fstream fp);
	~Cache();
	uint readBlock(usint address);
	usint readByte(usint address);
	void writeBlock(usint address, uint data);
	void writeByte(usint address, uint data);
	void resetAccesses();
	bool readBusy();
	bool writeBusy();
	void updateSrcFile();
};
Cache::Cache(std::fstream fp){
	this->srcFile = fp;
	sets = vector<uint> (cacheSize,0);
	std::string hexCode;
	uint value;
	int blockNum = 0;
	int offset = 0;
	while(fp >> hexCode){
		if(offset == 4)
		{
			offset = 0;
			blockNum++;
		}
		value = std::stoi(hexCode,0,16);
		for(int i = 0; i < offset; i++)
		{
			value << 8;
		}
		sets[blockNum] += value;
		offset++;
	}
}
uint Cache::readBlock(usint address){
	num_of_reads++;
	return sets[address >> 2];
}
usint Cache::readByte(usint address){
	uint data = readBlock(address);
	uint offset = address & (blockSize-1);
	for(int i = offset; i < 3; i++)
	{
		data << 8;
	}
	for(int i = 0; i < 3; i++)
	{
		data >> 8;
	}
	return (usint)data;
}
void Cache::writeBlock(usint address, uint data){
	sets[address >> log2(blockSize)] = data;
}
void Cache::writeByte(usint address, uint data){
	num_of_writes++;
	uint blockNum = address >> 2;
	uint offset = address & (blockSize-1);
	uint mask = cacheSize-1;
	for(int i = 0; i < offset; i++)
	{
		mask << 8;
		data << 8;
	}
	mask = UINT_MAX - mask;
	sets[blockNum] = (sets[blockNum] & mask) + data;
}
void Cache::resetAccesses(){
	num_of_reads = 0;
	num_of_writes = 0;
}
 bool Cache::readBusy(){
 	if(num_of_reads == 1)
 	{
 		return true;
 	}
 	return false;
 }
 bool Cache::writeBusy(){
 	if(num_of_writes == 1)
 	{
 		return true;
 	}
 	return false;
 }






class RegFile
{
	int regSize = 16;
	std::fstream srcFile;
	std::vector<usint> RF;
	int num_of_reads = 0;
	int num_of_writes = 0;
public:
	RegFile(std::fstream fp);
	~RegFile();
	usint read(usint index);
	void write(usint index, usint data);
	void updateSrcFile();
	void resetAccesses();
	bool readBusy();
	bool writeBusy();
};
RegFile::RegFile(std::fstream fp){
	this->srcFile = fp;
	RF = vector<uint> (regSize);
	std::string hexCode;
	usint value;
	int regNum = 0;
	while(fp >> hexCode){
		value = std::stoi(hexCode,0,16);
		RF[regNum] = value;
		regNum++;
	}
};
usint RegFile::read(usint index){
	num_of_reads++;
	return RF[index];
}
void RegFile::write(usint index,usint data){
	num_of_writes++;
	RF[index] = data;
}
void RegFile::resetAccesses(){
	num_of_reads = 0;
	num_of_writes = 0;
}
 bool RegFile::readBusy(){
 	if(num_of_reads == 2)
 	{
 		return true;
 	}
 	return false;
 }
 bool RegFile::writeBusy(){
 	if(num_of_writes == 1)
 	{
 		return true;
 	}
 	return false;
 }



class Processor  {
private:
	Cache* iCache, dCache;
	RegFile regFile;

	bool halted = false;

	bool stallIF = false;
	bool stallID = false;
	bool stallEX = false;
	bool stallMM = false;
	bool stallWB = false;

	int numRegistersRead = 0; 		// < 3
	bool numRegistersWritten = 0;	// < 2
	bool numMemReads = 0; 			// < 2
	bool numMemWrites = 0; 			// < 2

	int stat_instruction_count = 0;
	int stat_instruction_count_arith = 0;
	int stat_instruction_count_logic = 0;
	int stat_instruction_count_data = 0;
	int stat_instruction_count_control = 0;
	int stat_instruction_count_halt = 0;
	int stat_cycles = 0;
	int stat_stalls = 0;
	int stat_stalls_data = 0;
	int stat_stalls_control = 0;

	// declare IF registers here
	void fetchStage();
	// declare ID registers here
	void decodeStage();
	// declare EX registers here
	void executeStage();
	// declare MM registers here
	void memoryStage();
	// declare WB registers here
	void writebackStage();

	void flushPipeline();

	void readRegisters(int reg1, int reg2);
	void writeRegister(int reg, byte val);
public:
	// init with caches and rf
	Processor(std::fstream Icache, std::fstream Dcache, std::fstream RegFile);
	~Processor();

	// initiates run, runs until halted
	void run();
	// run one cycle
	void cycle();

	// is processor halted?
	bool isHalted();
};



int main()
{
	std::ifstream Icache, Dcache, RegFile;	//creating objects for file handling

	Icache.open("ICache.txt");	//getting the filepointer of the Instruction cache file
	Dcache.open("DCache.txt");	//getting the filepointer of the data cache file
	RegFile.open("RF.txt");	//the input for the register file

	$className$ processor(Icache, Dcache, RegFile);	//sending the adress class as pointers/*change class name*/
	processor.process();	/*change method name*/
	return 0;	//exiting the code
}