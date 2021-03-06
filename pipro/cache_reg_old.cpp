#include <iostream>	//including all the required header files
#include <fstream>
#include <cassert>
#include <iterator>
#include <set>
#include <cstring>
#include <vector>
#include <climits>

/*-------------------------------------------------------------------------------------------------
*    Author   : Team DOOFENSMARTZ
*    Code     : CPP code to simulate a pipeline processor
*    Question : CS2610 A8
-------------------------------------------------------------------------------------------------*/

typedef unsigned int uint;	//using typedef to change names for easier use
typedef unsigned short usint;
typedef unsigned char byte;

#define IF 0	//making definitions to easen the code notations
#define ID 1
#define EX 2
#define MM 3
#define WB 4

#define OPC_ADD 0	//making definitions to easen the code notations
#define OPC_SUB 1
#define OPC_MUL 2
#define OPC_INC 3
#define OPC_AND 4
#define OPC_OR 5
#define OPC_NOT 6
#define OPC_XOR 7
#define OPC_LD 8
#define OPC_ST 9
#define OPC_JMP 10
#define OPC_BEQZ 11
#define OPC_HALT 15


int log2(uint x)	//a function to find value of log of a number wrt base 2
{
    int r = 0;
    while(x > 0)	
    {
        x = x >> 1;	//performing repeated right shifts to reduce the number by 2 in each step
        r++;	//counting the number of right shifts
    }

    return r - 1;	//returing the value of log
}

int pow2(uint n)	//a function to find value of 2 raised to the power of a number
{
    int r = 1;
    while(n > 0)
    {
        r = r << 1;	//performing repeated left shifts to multiply by 2 in each step
        n--;	//decrimentiing the count
    }

    return r;	//returning the result
}

/****************************************************************************************************
 * 	Class Name	: Cache
 * 	Inheritences: NIL
 * 	Use			: Used to act as a cache for a processor
****************************************************************************************************/
class Cache	//the cache class that is being used for Data and instrution caches
{
	uint cacheSize = 256;
	uint blockSize = 4;
	std::ifstream * srcFile;
	std::vector<uint> sets;
	int num_of_reads = 0;
	int num_of_writes = 0;
	//Little Endian
public:
	Cache(std::ifstream * fp);
	~Cache();
	uint readBlock(byte address);
	byte readByte(byte address);
	void writeBlock(byte address, uint data);
	void writeByte(byte address, byte data);
	void resetAccesses();
	bool readBusy();
	bool writeBusy();
	void updateSrcFile();
};
Cache::Cache(std::ifstream * fp){
	this->srcFile = fp;
	sets = std::vector<uint> (cacheSize, 0);

	std::string hexCode;
	uint value;
	int blockNum = 0;
	int offset = 0;
	while(*fp >> hexCode){
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
uint Cache::readBlock(byte address){
	num_of_reads++;
	return sets[address >> 2];
}
byte Cache::readByte(byte address){
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
	return (byte)data;
}
void Cache::writeBlock(byte address, uint data){
	sets[address >> log2(blockSize)] = data;
}
void Cache::writeByte(byte address, byte data){
	num_of_writes++;
	byte blockNum = address >> 2;
	byte offset = address & (blockSize-1);
	uint mask = cacheSize-1;
	for(int i = 0; i < offset; i++)
	{
		mask << 8;
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

/****************************************************************************************************
 * 	Class Name	: RegFile
 * 	Inheritences: NIL
 * 	Use			: Used to act as a the register file for the processor
****************************************************************************************************/
class RegFile
{
	int regSize = 16;
	std::ifstream * srcFile;
	std::vector<byte> RF;
	std::vector<bool> status;
	int num_of_reads = 0;
	int num_of_writes = 0;
public:
	RegFile(std::ifstream * fp);
	~RegFile();
	byte read(byte index);
	void write(byte index, byte data);
	void updateSrcFile();
	void resetAccesses();
	bool readBusy();
	bool writeBusy();
	void setStatus(byte index, bool currStatus);
	bool isOpen(byte index);
};

RegFile::RegFile(std::ifstream * fp){
	this->srcFile = fp;
	RF = std::vector<byte> (regSize);
	status = std::vector<bool> (regSize,false);
	std::string hexCode;
	byte value;
	int regNum = 0;
	while(*fp >> hexCode){
		value = std::stoi(hexCode,0,16);
		RF[regNum] = value;
		regNum++;
	}
};

byte RegFile::read(byte index){
	num_of_reads++;
	return RF[index];
}
void RegFile::write(byte index,byte data){
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

bool RegFile::isOpen(byte index){
	return status[index];
}
void RegFile::setStatuset(byte index, bool currStatus){
	status[index] = currStatus;
}
/****************************************************************************************************
 * 	Class Name	: Processor
 * 	Inheritences: NIL
 * 	Use			: Used to act as the pipeline processor for the simulator
****************************************************************************************************/
class Processor  {
private:
	Cache* iCache, dCache;	//cache object pointers to store instruction and data caches
	RegFile* regFile;	//register file object pointer to allot the object later

	bool haltScheduled = false;
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
	Processor(std::ifstream * Icache, std::ifstream * Dcache, std::ifstream * RegFile);
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

Processor::Processor(std::ifstream * Icache, std::ifstream * Dcache, std::ifstream * regFile)  {
	iCache = new Cache(Icache);
	dCache = new Cache(Dcache);
	this->regFile = new RegFile(regFile);
}

void Processor::run()  {	//the run function to initiate the processor
	while(!haltScheduled && !isHalted())  {	//while halt is not called execute instructions
		cycle();	//call the cycle function, the equivalent of one cycle of the processor
	}

	while(!haltScheduled)  {
		if((EX_run || MM_run || WB_run) == false)  {
			halted = true;
			haltScheduled = false;
			return;
		}
	}
}

void Processor::cycle()  {
	// to simulate parallelism we execute the pipeline stages
	// in the order WB MEM EX ID IF in one cycle,
	// so WB's buffer is cleared before MEM tries
	// to write to it
	stat_cycles++;

	writebackStage();
	memoryStage();
	executeStage();
	decodeStage();
	fetchStage();
}

bool Processor::isHalted()  {
	return halted;
}


void Processor::executeStage()
{
	int opCode = REG_EX_IR >> 12;
	
	switch(opCode)
	{
		case OPC_ADD :	REG_MM_AO = REG_EX_A + REG_EX_B; break;
		case OPC_SUB :	REG_MM_AO = REG_EX_A - REG_EX_B; break;
		case OPC_MUL :	REG_MM_AO = REG_EX_A * REG_EX_B; break;
		case OPC_INC :	REG_MM_AO = REG_EX_A + 1; 		 break;
		case OPC_AND :	REG_MM_AO = REG_EX_A & REG_EX_B; break;
		case OPC_OR :	REG_MM_AO = REG_EX_A | REG_EX_B; break;
		case OPC_NOT :	REG_MM_AO = ~REG_EX_A; 			 break;
		case OPC_XOR :	REG_MM_AO = REG_EX_A ^ REG_EX_B; break;
		case OPC_LD :	REG_MM_AO = REG_EX_A + REG_EX_B; break;
		case OPC_ST :	REG_MM_AO = REG_EX_A + REG_EX_B; break;

		case OPC_JMP:
			stallEX = true;
			REG_MM_AO = REG_EX_PC + //fix the jump statement//
			break;

		case OPC_BEQZ:	if(REG_EX_A==0)
						REG_MM_AO =  //fix the jump statement//
					break;

		default:	break;
	}

	if((opCode==8) || (opCode==9))
	{
		MM_run = true;	//move to memory operation
		REG_MM_IR = REG_EX_IR;	//passing the instruction value
	}

	else
	{
		WB_run = true;	//move to writeback operation
		REG_WB_AO = REG_MM_AO;
		REG_WB_IR = REG_EX_IR;
	}

	EX_run = false;	//setting that the process is finished
}

void Processor::fetchStage()  {
	// wait for ID to be available
	if(ID_run)  { // due to backward nature, if ID_run is true, it has been stalling
		IF_run = true;
		return;
	}

	REG_ID_IR = (((usint) iCache->readByte(REG_IF_PC)) << 8) + ((usint) iCache->readByte(REG_IF_PC));

	ID_run = true;

	REG_IF_PC += 2;
}

void Processor::decodeStage()  {
	if((!ID_run) || stallID)  {
		return;
	}

	if(EX_run)  {
		ID_run = true;
		return;
	}

	REG_EX_IR = REG_ID_IR;

	byte opcode = (REG_ID_IR & 0xf000) >> 12;
	
	// halt class
	if(opcode == OPC_HALT)  {
		IF_run = false;
		ID_run = false;

		haltScheduled = true;
		// let the other pipeline stages (previous instructions) complete
		return;
	}

	ID_run = false;
	EX_run = true;

	// control class - HAZ
	if(opcode == OPC_JMP)  {
		stallID = true;
		
		REG_EX_A = (REG_ID_IR & 0x0ff0) >> 4;
		return;
	}
	if(opcode == OPC_BEQZ)  {
		stallID = true;
		addr1 = (REG_ID_IR & 0x0f00) >> 8;
		if(regFile->isOpen(addr1))
		{
			EX_run = false;
			ID_run = true;
			return;
		}
		REG_EX_A = regFile->read((REG_ID_IR & 0x0f00) >> 8);
		REG_EX_B = REG_ID_IR & 0x00ff;
		return;
	}
	
	usint destReg = (REG_ID_IR & 0x0f00) >> 8;
	usint addr1;
	usint addr2;
	regFile->setStatus((byte)destReg, true);
	// data class
	if((opcode == OPC_LD))  {
		addr1 = (REG_ID_IR & 0x00f0) >> 4;
		if(regFile->isOpen(addr1))
		{
			EX_run = false;
			ID_run = true;
			return;
		}
		REG_EX_A = regFile->read((REG_ID_IR & 0x00f0) >> 4);
		REG_EX_B = REG_ID_IR & 0x000f;
		return;
	}
	if( (opcode == OPC_ST))  {
		addr1 = (REG_ID_IR & 0x00f0) >> 4;
		addr2 = REG_ID_IR & 0x000f;
		if(regFile->isOpen(addr1) || regFile->isOpen(addr2))
		{
			EX_run = false;
			ID_run = true;
			return;
		}
		REG_EX_A = regFile->read((REG_ID_IR & 0x00f0) >> 4);
		REG_EX_B = REG_ID_IR & 0x000f;
		return;
	}

	// arithmetic class - HAZ
	if((opcode >= OPC_ADD) && (opcode <= OPC_MUL))  {
		addr1 = (REG_ID_IR & 0x00f0) >> 4;
		addr2 = REG_ID_IR & 0x000f;
		if(regFile->isOpen(addr1) || regFile->isOpen(addr2))
		{
			EX_run = false;
			ID_run = true;
			return;
		}
		REG_EX_A = regFile->read((REG_ID_IR & 0x00f0) >> 4);
		REG_EX_B = regFile->read(REG_ID_IR & 0x000f);
		return;
	}
	if(opcode == OPC_INC)  {
		addr1 = (REG_ID_IR & 0x0f00) >> 8;
		if(regFile->isOpen(addr1))
		{
			EX_run = false;
			ID_run = true;
			return;
		}
		REG_EX_A = regFile->read((REG_ID_IR & 0x0f00) >> 8);
		return;
	}

	// logical class - HAZ
	if((opcode != OPC_NOT))  {
		addr1 = (REG_ID_IR & 0x00f0) >> 4;
		addr2 = REG_ID_IR & 0x000f;
		if(regFile->isOpen(addr1) || regFile->isOpen(addr2))
		{
			EX_run = false;
			ID_run = true;
			return;
		}
		REG_EX_A = regFile->read((REG_ID_IR & 0x00f0) >> 4);
		REG_EX_B = regFile->read(REG_ID_IR & 0x000f);
		return;
	}

	// here, opcode == OPC_NOT
	if(opcode == OPC_NOT)	{
		addr1 = (REG_ID_IR & 0x00f0) >> 4;
			if(regFile->isOpen(addr1))
			{
				EX_run = false;
				ID_run = true;
				return;
			}
		REG_EX_A = regFile->read((REG_ID_IR & 0x00f0) >> 4);
	}
}


void Processor::memoryStage(){
	if(!MM_run || stallMM)
	{
		return;
	}
	usint opCode = REG_MM_IR >> 12;
	if(opCode == 9)
	{
		dCache->writeByte(REG_MM_AO,REG_EX_B);
		numMemWrites++;
	}
	else if(opCode == 8)
	{
		REG_WB_LMD = dCache->readByte(REG_MM_AO);
		REG_WB_IR = REG_MM_IR;
		REG_WB_AO = REG_MM_AO;
		WB_run = true;
		numMemReads++;
	}
	MM_run = false;
	return;
}

void Processor::writebackStage(){
	if(!WB_run || stallWB)
	{
		return;
	}
	usint opCode = REG_WB_IR >> 12;
	usint offset = REG_WB_IR << 4;
	offset = offset >> 12;
	if(opCode == 8) 
	{
		regFile->write((byte)offset, REG_WB_LMD);
	}
	else 
	{
		regFile->write((byte)offset, REG_WB_AO);
	}
	regFile->setStatus((byte)offset, false);
	MM_run = false;
}

/*-------------------------------------------------------------------------------------------------
*    Function Name : main
*    Args          : Nil
*    Return Type   : int(0)
*    Application   : Entry point to the Proram
-------------------------------------------------------------------------------------------------*/
int main()
{
	std::ifstream Icache, Dcache, RegFile;	//creating objects for file handling

	Icache.open("ICache.txt");	//getting the filepointer of the Instruction cache file
	Dcache.open("DCache.txt");	//getting the filepointer of the data cache file
	RegFile.open("RF.txt");	//the input for the register file

	Processor processor(&Icache, &Dcache, &RegFile);	//sending the adress class as pointers/*change class name*/
	processor.run();	//running the processor
	return 0;	//exiting the code
}