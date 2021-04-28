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

	// declare IF registers here
	byte REG_IF_PC = 0u; // first instruction
	bool IF_run = true;
	void fetchStage();

	// declare ID registers here
	usint REG_ID_IR = 0u; // add R0 to R0 and store in R0
	byte  REG_ID_PC = 0u;
	bool  ID_run = false;
	void decodeStage();

	// declare EX registers here
	usint REG_EX_IR = 0u;
	byte  REG_EX_PC = 0u;
	byte  REG_EX_A  = 0u;
	byte  REG_EX_B  = 0u;
	bool  EX_run = false;
	void executeStage();

	// declare MM registers here
	usint REG_MM_IR = 0u;
	byte  REG_MM_AO = 0u;
	bool  MM_run = false;
	void memoryStage();

	// declare WB registers here
	usint REG_WB_IR   = 0u;
	byte  REG_WB_AO   = 0u;
	byte  REG_WB_LMD  = 0u;
	bool  REG_WB_COND = false;
	bool  WB_run = false;
	void writebackStage();

	void flushPipeline();

	void readRegisters(int reg1, int reg2);
	void writeRegister(int reg, byte val);
public:
	// init with caches and rf
	Processor(std::fstream& Icache, std::fstream& Dcache, std::fstream& RegFile);
	~Processor();

	// initiates run, runs until halted
	void run();
	// run one cycle
	void cycle();

	// is processor halted?
	bool isHalted();

	// stats
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
};

Processor::Processor(std::fstream Icache, std::fstream Dcache, std::fstream RegFile)  {
	iCache = new Cache(Icache);
	dCache = new Cache(dCache);
	regFile = new Cache(RegFile);
}

void Processor::run()  {
	while(!isHalted())  {
		cycle();
	}
}

void Processor::cycle()  {
	// to simulate parallelism we execute the pipeline stages
	// in the order WB MEM EX ID IF in one cycle,
	// so WB's buffer is cleared before MEM tries
	// to write to it
	stat_cycles++;
}

bool Processor::isHalted()  {
	return halted;
}

void Processor::executeStage()
{
	int opCode = REG_EX_IR >> 12;
	
	switch(opCode)
	{
		case 0 :	REG_MM_AO = REG_EX_A + REG_EX_B;	//setting the result
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		case 1 :	REG_MM_AO = REG_EX_A - REG_EX_B;	//setting the result
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		case 2 :	REG_MM_AO = REG_EX_A * REG_EX_B;	//setting the result
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		case 3 :	REG_MM_A0 = REG_EX_A + 1;	//setting the result
					break;	//break statement for the switch

		case 4 :	REG_MM_AO = REG_EX_A & REG_EX_B;	//setting the result
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		case 5 :	REG_MM_AO = REG_EX_A | REG_EX_B;	//setting the result
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		case 6 :	REG_MM_AO = ~REG_EX_A;	//setting the result
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		case 7 :	REG_MM_AO = REG_EX_A ^ REG_EX_B;	//setting the result
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		case 8 :	REG_MM_AO = REG_EX_A + REG_EX_B;	//setting the result
					MM_run = true;	//operate memory next
					break;	//break statement for the switch

		case 9 :	REG_MM_AO = REG_EX_A + REG_EX_B;	//setting the result
					MM_run = true;	//operate memory next
					break;	//break statement for the switch

		case 10:	REG_MM_AO = REG_EX_PC + //fix the jump statement//
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		case 11:	if(REG_EX_A==0)
						REG_MM_AO =  //fix the jump statement//
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		case 15:	REG_MM_AO = 
					WB_run = true;	//skip to write back
					break;	//break statement for the switch

		default:	break;	//break statement for the switch
	}

	EX_run = false;	//setting that the process is finished
}

int main()
{
	std::ifstream Icache, Dcache, RegFile;	//creating objects for file handling

	Icache.open("ICache.txt");	//getting the filepointer of the Instruction cache file
	Dcache.open("DCache.txt");	//getting the filepointer of the data cache file
	RegFile.open("RF.txt");	//the input for the register file

	Processor processor(Icache, Dcache, RegFile);	//sending the adress class as pointers/*change class name*/
	processor.run();	//running the processor
	return 0;	//exiting the code
}