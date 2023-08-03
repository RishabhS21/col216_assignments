/**
 * @file MIPS_Processor.hpp
 * @author Mallika Prabhakar and Sayam Sethi
 * 
 */

#ifndef __MIPS_PROCESSOR_HPP__
#define __MIPS_PROCESSOR_HPP__

#include <set>
#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <fstream>
#include <exception>
#include <iostream>
#include <boost/tokenizer.hpp>

struct MIPS_Architecture
{
	int registers[32] = {0}, PCcurr = 0, PCnext;
	std::unordered_map<std::string, std::function<int(MIPS_Architecture &, std::string, std::string, std::string)>> instructions;
	std::unordered_map<std::string, int> registerMap, address;
	static const int MAX = (1 << 20);
	int data[MAX >> 2] = {0};
	std::unordered_map<int, int> memoryDelta,memoryDelta1;
	std::vector<std::vector<std::string>> commands;
	std::vector<int> commandCount;
	enum exit_code
	{
		SUCCESS = 0,
		INVALID_REGISTER,
		INVALID_LABEL,
		INVALID_ADDRESS,
		SYNTAX_ERROR,
		MEMORY_ERROR
	};
	std::set<std::string>R_type={"add","sub","mul","slt","and","or"};
	std::set<std::string>I_type={"lw","sw","addi","beq","bne"};
	std::set<std::string>J_type={"j"};

	struct _5stagepipeline_registers{		//this is a structure for defining all the pipeline registers which are used as intermediate in pipeline process
		struct IF_ID{
			int PC=0;				//initialsing different variables with empty or zero values..
			std::vector<std::string> instruction = {"","","",""};
		}if_id;
		struct ID_EX{
			int PC,r_data1,r_data2,shamt,load_word;
			std::string instruction15_0,instruction20_16,instruction15_11,instruction;
		}id_ex;
		struct EX_MEM{
			int PC,zero,ALU_result,w_data,lw_bypass;
			std::string instruction,base_instruction,PC_branch;
		}ex_mem;
		struct MEM_WB{
			int data,PC;
			std::string instruction,ba_instruction;
		}mem_wb;
		struct PipelineControl{
			int RegDst=0,ALUop=0,ALUsrc=0,Branch=0,Mem_read=0,Mem_write=0,Reg_write=0,Memtoreg=0,PCsrc=0;
		}pipelinecontrol;

	};


	struct _79stagepipeline{
		struct IF1_IF2{
			int PC=-1;
			std::vector<std::string> instruction = {"","","",""};
		}if1_if2;
		struct IF2_DEC1{
			int PC=0;
			std::vector<std::string> instruction={"","","",""};
		}if2_dec1;
		struct DEC1_DEC2{
			int PC=-1;
			std::vector<std::string> instruction={"","","",""};;
		}dec1_dec2;
		struct DEC2_RR{
			int PC=-1,r_data1,r_data2;
			std::string instruction15_0,instruction20_16,instruction15_11,instruction;
		}dec2_rr;
		struct RR_EX1{
			int PC=-1,r_data1,r_data2;
			std::string instruction15_0,instruction20_16,instruction15_11,instruction;
		}rr_ex1;
		struct EX1_RW1{
			int PC=-1,ALUresut;
			std::string rd_register,PC_branch,base_instruction;
		}ex1_rw1;
		struct RR_EX2{
			int PC=-1,r_data1,r_data2;
			std::string instruction15_0,instruction20_16,instruction;
		}rr_ex2;
		struct EX2_DM1{
			int PC=-1,address,sw_data;
			std::string rs_register,base_instruction;
		}ex2_dm1;
		struct DM1_DM2{
			int PC=-1,address,sw_data;
			std::string rs_register,base_instruction;
		}dm1_dm2;
		struct DM2_RW2{
			int PC=-1,lw_data;
			std::string lw_register,base_instruction;
		}dm2_rw2;
		struct Last{
			int PC=0-1;
		}last;
		struct controls{
			int PCsrc=0;
		} pipelinecontrol;
	};
	// constructor to initialise the instruction set
	MIPS_Architecture(std::ifstream &file)
	{
		instructions = {{"add", &MIPS_Architecture::add}, {"sub", &MIPS_Architecture::sub}, {"mul", &MIPS_Architecture::mul}, {"beq", &MIPS_Architecture::beq}, {"bne", &MIPS_Architecture::bne}, {"slt", &MIPS_Architecture::slt}, {"j", &MIPS_Architecture::j}, {"lw", &MIPS_Architecture::lw}, {"sw", &MIPS_Architecture::sw}, {"addi", &MIPS_Architecture::addi}};

		for (int i = 0; i < 32; ++i)
			registerMap["$" + std::to_string(i)] = i;
		registerMap["$zero"] = 0;
		registerMap["$at"] = 1;
		registerMap["$v0"] = 2;
		registerMap["$v1"] = 3;
		for (int i = 0; i < 4; ++i)
			registerMap["$a" + std::to_string(i)] = i + 4;
		for (int i = 0; i < 8; ++i)
			registerMap["$t" + std::to_string(i)] = i + 8, registerMap["$s" + std::to_string(i)] = i + 16;
		registerMap["$t8"] = 24;
		registerMap["$t9"] = 25;
		registerMap["$k0"] = 26;
		registerMap["$k1"] = 27;
		registerMap["$gp"] = 28;
		registerMap["$sp"] = 29;
		registerMap["$s8"] = 30;
		registerMap["$ra"] = 31;

		constructCommands(file);
		commandCount.assign(commands.size(), 0);
	}
	// perform add operation
	int add(std::string r1, std::string r2, std::string r3)
	{
		return op(r1, r2, r3, [&](int a, int b)
				  { return a + b; });
	}

	// perform subtraction operation
	int sub(std::string r1, std::string r2, std::string r3)
	{
		return op(r1, r2, r3, [&](int a, int b)
				  { return a - b; });
	}

	// perform multiplication operation
	int mul(std::string r1, std::string r2, std::string r3)
	{
		return op(r1, r2, r3, [&](int a, int b)
				  { return a * b; });
	}

	// perform the binary operation
	int op(std::string r1, std::string r2, std::string r3, std::function<int(int, int)> operation)
	{
		if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
			return 1;
		registers[registerMap[r1]] = operation(registers[registerMap[r2]], registers[registerMap[r3]]);
		PCnext = PCcurr + 1;
		return 0;
	}

	// perform the beq operation
	int beq(std::string r1, std::string r2, std::string label)
	{
		return bOP(r1, r2, label, [](int a, int b)
				   { return a == b; });
	}

	// perform the bne operation
	int bne(std::string r1, std::string r2, std::string label)
	{
		return bOP(r1, r2, label, [](int a, int b)
				   { return a != b; });
	}

	// implements beq and bne by taking the comparator
	int bOP(std::string r1, std::string r2, std::string label, std::function<bool(int, int)> comp)
	{
		if (!checkLabel(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		if (!checkRegisters({r1, r2}))
			return 1;
		PCnext = comp(registers[registerMap[r1]], registers[registerMap[r2]]) ? address[label] : PCcurr + 1;
		return 0;
	}

	// implements slt operation
	int slt(std::string r1, std::string r2, std::string r3)
	{
		if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
			return 1;
		registers[registerMap[r1]] = registers[registerMap[r2]] < registers[registerMap[r3]];
		PCnext = PCcurr + 1;
		return 0;
	}

	// perform the jump operation
	int j(std::string label, std::string unused1 = "", std::string unused2 = "")
	{
		if (!checkLabel(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		PCnext = address[label];
		return 0;
	}

	// perform load word operation
	int lw(std::string r, std::string location, std::string unused1 = "")
	{
		if (!checkRegister(r) || registerMap[r] == 0)
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);
		registers[registerMap[r]] = data[address];
		PCnext = PCcurr + 1;
		return 0;
	}

	// perform store word operation
	int sw(std::string r, std::string location, std::string unused1 = "")
	{
		if (!checkRegister(r))
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);
		if (data[address] != registers[registerMap[r]])
			memoryDelta[address] = registers[registerMap[r]];
		data[address] = registers[registerMap[r]];
		PCnext = PCcurr + 1;
		return 0;
	}

	int locateAddress(std::string location)
	{
		if (location.back() == ')')
		{
			try
			{
				int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
				std::string reg = location.substr(lparen + 1);
				reg.pop_back();
				if (!checkRegister(reg))
					return -3;
				int address = registers[registerMap[reg]] + offset;
				if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
					return -3;
				return address / 4;
			}
			catch (std::exception &e)
			{
				return -4;
			}
		}
		try
		{
			int address = stoi(location);
			if (address % 4 || address < int(4 * commands.size()) || address >= MAX)
				return -3;
			return address / 4;
		}
		catch (std::exception &e)
		{
			return -4;
		}
	}

	// perform add immediate operation
	int addi(std::string r1, std::string r2, std::string num)
	{
		if (!checkRegisters({r1, r2}) || registerMap[r1] == 0)
			return 1;
		try
		{
			registers[registerMap[r1]] = registers[registerMap[r2]] + stoi(num);
			PCnext = PCcurr + 1;
			return 0;
		}
		catch (std::exception &e)
		{
			return 4;
		}
	}

	// checks if label is valid
	inline bool checkLabel(std::string str)
	{
		return str.size() > 0 && isalpha(str[0]) && all_of(++str.begin(), str.end(), [](char c)
														   { return (bool)isalnum(c); }) &&
			   instructions.find(str) == instructions.end();
	}

	// checks if the register is a valid one
	inline bool checkRegister(std::string r)
	{
		return registerMap.find(r) != registerMap.end();
	}

	// checks if all of the registers are valid or not
	bool checkRegisters(std::vector<std::string> regs)
	{
		return std::all_of(regs.begin(), regs.end(), [&](std::string r)
						   { return checkRegister(r); });
	}

	/*
		handle all exit codes:
		0: correct execution
		1: register provided is incorrect
		2: invalid label
		3: unaligned or invalid address
		4: syntax error
		5: commands exceed memory limit
	*/
	void handleExit(exit_code code, int cycleCount)
	{
		std::cout << '\n';
		switch (code)
		{
		case 1:
			std::cerr << "Invalid register provided or syntax error in providing register\n";
			break;
		case 2:
			std::cerr << "Label used not defined or defined too many times\n";
			break;
		case 3:
			std::cerr << "Unaligned or invalid memory address specified\n";
			break;
		case 4:
			std::cerr << "Syntax error encountered\n";
			break;
		case 5:
			std::cerr << "Memory limit exceeded\n";
			break;
		default:
			break;
		}
		if (code != 0)
		{
			std::cerr << "Error encountered at:\n";
			for (auto &s : commands[PCcurr])
				std::cerr << s << ' ';
			std::cerr << '\n';
		}
	}

	// parse the command assuming correctly formatted MIPS instruction (or label)
	void parseCommand(std::string line)
	{
		// strip until before the comment begins
		line = line.substr(0, line.find('#'));
		std::vector<std::string> command;
		boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
		for (auto &s : tokens)
			command.push_back(s);
		// empty line or a comment only line
		if (command.empty())
			return;
		else if (command.size() == 1)
		{
			std::string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command.clear();
		}
		else if (command[0].back() == ':')
		{
			std::string label = command[0].substr(0, command[0].size() - 1);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command = std::vector<std::string>(command.begin() + 1, command.end());
		}
		else if (command[0].find(':') != std::string::npos)
		{
			int idx = command[0].find(':');
			std::string label = command[0].substr(0, idx);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command[0] = command[0].substr(idx + 1);
		}
		else if (command[1][0] == ':')
		{
			if (address.find(command[0]) == address.end())
				address[command[0]] = commands.size();
			else
				address[command[0]] = -1;
			command[1] = command[1].substr(1);
			if (command[1] == "")
				command.erase(command.begin(), command.begin() + 2);
			else
				command.erase(command.begin(), command.begin() + 1);
		}
		if(command[0]=="sw" || command[0]=="lw"){
			std::string next=command[2];
			std::string j="";
			for(auto x:next){
				if(x=='('){
					command.push_back(j);
					j="";
				}
				else if(x==')'){
					command[2]=j;
					j="";
					break;
				}
				else{
					j=j+x;
				}
			}
		}
		if (command.empty())
			return;
		if (command.size() > 4)
			for (int i = 4; i < (int)command.size(); ++i)
				command[3] += " " + command[i];
		command.resize(4);
		if(command[0]=="sw" || command[0]=="lw"){
			std::string next=command[2];
			std::string j="";
			for(auto x:next){
				if(x=='('){
					command.push_back(j);
					j="";
				}
				else if(x==')'){
					command[2]=j;
					j="";
					break;
				}
				else{
					j=j+x;
				}
			}
		}
		commands.push_back(command);
	}

	// construct the commands vector from the input file
	void constructCommands(std::ifstream &file)
	{
		std::string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();
	}
	void printRegistersAndMemoryDelta(int clockCycle)
	{
		for (int i = 0; i < 32; ++i)
			std::cout << registers[i] << ' ';
		std::cout << '\n';
		std::cout << memoryDelta1.size() << ' ';
		for (auto &p : memoryDelta1)
			std::cout << p.first << ' ' << p.second ;
		std::cout<<"\n";
		memoryDelta1.clear();
	}
	// execute the commands sequentially (no pipelining)
	void executeCommandsUnpipelined()
	{
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		}

		int clockCycles = 0;
		while (PCcurr < commands.size())
		{
			++clockCycles;
			std::vector<std::string> &command = commands[PCcurr];
			if (instructions.find(command[0]) == instructions.end())
			{
				handleExit(SYNTAX_ERROR, clockCycles);
				return;
			}
			exit_code ret = (exit_code) instructions[command[0]](*this, command[1], command[2], command[3]);
			if (ret != SUCCESS)
			{
				handleExit(ret, clockCycles);
				return;
			}
			++commandCount[PCcurr];
			PCcurr = PCnext;
			printRegistersAndMemoryDelta(clockCycles);
		}
		handleExit(SUCCESS, clockCycles);
	}



	void execute5stagepipeline(){
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		}
		int clockCycles = 0;
		struct _5stagepipeline_registers pipeline;
		int input=0;
		
		while(PCcurr<commands.size() || pipeline.if_id.PC<commands.size() || pipeline.id_ex.PC<commands.size() || pipeline.ex_mem.PC<commands.size() || pipeline.mem_wb.PC<commands.size()){
			// std::cout<<"---"<<"\n";
			printRegistersAndMemoryDelta(clockCycles);
			clockCycles++;
			// std::cout<<"clock"<<clockCycles<<"\n";
			// FIRST HALF


			//WB(write back)
			std::string c_instruction=pipeline.mem_wb.ba_instruction;
			// std::cout<<pipeline.mem_wb.PC<<c_instruction<<"\n";
			if(R_type.count(c_instruction)!=0 || c_instruction=="lw" || c_instruction=="addi"){
				std::string writeregister=pipeline.mem_wb.instruction;
				int registerdata=pipeline.mem_wb.data;
				registers[registerMap[writeregister]]=registerdata;
			}

			//MEM(memory write stage)
			int address1;
			// std::cout<<pipeline.ex_mem.PC<<pipeline.ex_mem.base_instruction<<"\n";
			int data;
			int PC_mem=pipeline.ex_mem.PC;
			int swdata;
			std::string b_instruction=pipeline.ex_mem.base_instruction;
			int Zero=pipeline.ex_mem.zero;
			std::string mem_instruction=pipeline.ex_mem.instruction;
			if(R_type.count(b_instruction)!=0 || b_instruction=="addi"){
				data=pipeline.ex_mem.ALU_result;
			}
			else if(b_instruction=="lw"){
				address1=pipeline.ex_mem.ALU_result;
				// std::cout<<address<<"\n";
				data=memoryDelta[address1/4];
				memoryDelta.erase(address1/4);
				// std::cout<<data<<"\n";
			}
			else if(b_instruction=="sw"){
				address1=pipeline.ex_mem.ALU_result;
				swdata=pipeline.ex_mem.w_data;
				memoryDelta[address1/4]=swdata;
				memoryDelta1[address1/4]=swdata;
			}


			//EX(Execution ALU)
			// std::cout<<pipeline.id_ex.PC<<pipeline.id_ex.instruction<<"\n";
			int PC1=pipeline.id_ex.PC;
			std::string PC_ex;
			int rdata1=pipeline.id_ex.r_data1;
			int rdata2=pipeline.id_ex.r_data2;
			int Alures;
			int writedata;
			std::string base=pipeline.id_ex.instruction;
			std::string ex_instruction;
			if(pipeline.id_ex.instruction=="add"){
				Alures=rdata1+rdata2;
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="sub"){
				Alures=rdata1-rdata2;
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="and"){
				Alures=rdata1 & rdata2;
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="slt"){
				if(rdata1<rdata2){
					Alures=1;
				}
				else{
					Alures=0;
				}
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="or"){
				Alures=rdata1 | rdata2;
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="mul"){
				Alures=rdata1*rdata2;
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="addi"){
				Alures=rdata1+stoi(pipeline.id_ex.instruction15_0);
				ex_instruction=pipeline.id_ex.instruction20_16;
			}
			else if(pipeline.id_ex.instruction=="sw"){
				Alures=rdata1+stoi(pipeline.id_ex.instruction15_0);
				writedata=rdata2;
			}
			else if(pipeline.id_ex.instruction=="lw"){
				// std::cout<<"1111"<<"\n";
				// std::cout<<pipeline.id_ex.instruction15_0<<"\n";
				Alures=rdata1+stoi(pipeline.id_ex.instruction15_0);
				ex_instruction=pipeline.id_ex.instruction20_16;
			}
			else if(pipeline.id_ex.instruction=="beq" || pipeline.id_ex.instruction=="bne"){
				Alures=rdata1-rdata2;
				PC_ex=(pipeline.id_ex.instruction15_0);
				if(base=="beq"){
					if(Alures==0){
						// std::cout<<"pp"<<"\n";
						pipeline.pipelinecontrol.PCsrc=1;
					
					}
				}
				else if(base=="bne"){
					if(Alures=0){
						pipeline.pipelinecontrol.PCsrc=1;
					}
				}


			}


			// ID{instruction decoder}
			// std::cout<<pipeline.if_id.PC<<pipeline.if_id.instruction[0]<<"\n";
			int PC_id=pipeline.if_id.PC;
			std::vector<std::string>id_instruction=pipeline.if_id.instruction;
			std::string read_register1;
			std::string read_register2;
			int read_data1;
			int read_data2;
			int jump=0;
			if(id_instruction[0]=="j"){
				jump=1;
			}
			if(R_type.count(id_instruction[0])!=0){
				read_register1=id_instruction[2];
				read_register2=id_instruction[3];
			}
			else if(I_type.count(id_instruction[0])!=0){
				read_register1=id_instruction[2];
				read_register2=id_instruction[1];
			}
			int datahazard=0;
			if((read_register1==ex_instruction && ex_instruction!="$0" && ex_instruction!="")){
				datahazard=1;
				id_instruction={"add","$0","$0","$0"};
				read_data1=0;
				read_data2=0;
			}
			else if((R_type.count(id_instruction[0])!=0 || id_instruction[0]=="sw" || id_instruction[0]=="beq" || id_instruction[0]=="bne") && read_register2==ex_instruction && ex_instruction!="$0" && ex_instruction!=""){
				datahazard=1;
				id_instruction={"add","$0","$0","$0"};
				read_data1=0;
				read_data2=0;
			}
			else if(read_register1==mem_instruction && mem_instruction!="$0" && mem_instruction!="") {
				datahazard=2;
				id_instruction={"add","$0","$0","$0"};
				read_data1=0;
				read_data2=0;
			}
			else if((R_type.count(id_instruction[0])!=0 || id_instruction[0]=="sw" || id_instruction[0]=="beq" || id_instruction[0]=="bne") && read_register2==mem_instruction && mem_instruction!="$0" && mem_instruction!=""){
				datahazard=2;
				id_instruction={"add","$0","$0","$0"};
				read_data1=0;
				read_data2=0;
			}
			else{
				read_data1=registers[registerMap[read_register1]];
				read_data2=registers[registerMap[read_register2]]; 
			}
			// std::cout<<"datahazard::"<<datahazard<<"\n";
			

			//IF(instruction fetch)
			// std::cout<< PCcurr <<"\n";
			int PC_if=PCcurr;
			int branch=0;
			std::vector<std::string>if_instruction;
			if(PC_if>=commands.size() || id_instruction[0]=="beq" || id_instruction[0]=="bne") {
				if_instruction={"add","$0","$0","$0"};
				branch=1;
			}
			else if(base=="beq" || base=="bne"){
				if_instruction={"add","$0","$0","$0"};
				branch=2;
			}
			else if(id_instruction[0]=="j"){
				if_instruction={"add","$0","$0","$0"};
			}
			else{
				if_instruction = commands[PC_if];
			}
			input=input+1;


			//SECOND HALF

			//MEM(memory stage)
			pipeline.mem_wb.data=data;
			pipeline.mem_wb.instruction=mem_instruction;
			pipeline.mem_wb.PC=PC_mem;
			pipeline.mem_wb.ba_instruction=b_instruction;
			//EX(Execution stage)
			if(Alures==0){
				pipeline.ex_mem.zero=1;
			}
			pipeline.ex_mem.ALU_result=Alures;
			pipeline.ex_mem.instruction=ex_instruction;
			pipeline.ex_mem.PC_branch=PC_ex;
			pipeline.ex_mem.PC=PC1;
			pipeline.ex_mem.w_data=writedata;
			pipeline.ex_mem.base_instruction=base;


			//ID(instruction decode)
			pipeline.id_ex.PC=PC_id;
			pipeline.id_ex.r_data1=read_data1;
			pipeline.id_ex.r_data2=read_data2;
			pipeline.id_ex.instruction=id_instruction[0];
			if(I_type.count(id_instruction[0])!=0){
				pipeline.id_ex.instruction15_0=id_instruction[3];
				pipeline.id_ex.instruction20_16=read_register2;
			}
			else if(R_type.count(id_instruction[0])!=0){
				pipeline.id_ex.instruction15_11=id_instruction[1];
			}


			//IF(instruction fetch)
			if (datahazard==0){

			pipeline.if_id.instruction=if_instruction;
			pipeline.if_id.PC=PC_if;
			if (instructions.find(pipeline.if_id.instruction[0]) == instructions.end())
				{
					handleExit(SYNTAX_ERROR, clockCycles);
					return;
				}
			}

		// making PCcurr value..
			if(jump==1){
				PCcurr=address[id_instruction[1]];
			}
			else if(datahazard==1 || datahazard==2 || branch==1){
				PCcurr=PCcurr;
			}
			else if(branch==2 && pipeline.pipelinecontrol.PCsrc==0){
				PCcurr=PCcurr;
			}
			else if(pipeline.pipelinecontrol.PCsrc==1){
				// std::cout<<"branch: "<<PC_ex<<"\n";
				PCcurr=address[PC_ex];
				pipeline.pipelinecontrol.PCsrc=0;
				if(PCcurr>=commands.size()){
					pipeline.pipelinecontrol.Branch=1;
				}if(c_instruction=="beq"||c_instruction=="bne"){
				if(pipeline.pipelinecontrol.Branch==1){
					break;
				}
			}
			}
			else{
				PCcurr++;
			}
			if(c_instruction=="beq"||c_instruction=="bne"){
				if(pipeline.pipelinecontrol.Branch==1){
					break;
				}
			}

		}
		printRegistersAndMemoryDelta(clockCycles);
	// std::cout<<address["loop"]<<"\n";
	// std::cout<<"Total Clockcycles:"<<clockCycles<<"\n";
	}


//BYPASSED 5 STAGE PIPELINE STARTS FROM HERE::::

	void execute5stage_bypasspipeline(){
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		}
		int clockCycles = 0;
		struct _5stagepipeline_registers pipeline;
		int input=0;
		
		while(PCcurr<commands.size() || pipeline.if_id.PC<commands.size() || pipeline.id_ex.PC<commands.size() || pipeline.ex_mem.PC<commands.size() || pipeline.mem_wb.PC<commands.size()){
			// std::cout<<"---"<<"\n";
			printRegistersAndMemoryDelta(clockCycles);
			clockCycles++;
			// std::cout<<"clock"<<clockCycles<<"\n";
			// FIRST HALF


			//WB(write back)
			std::string c_instruction=pipeline.mem_wb.ba_instruction;
			// std::cout<<pipeline.mem_wb.PC<<c_instruction<<"\n";
			if(R_type.count(c_instruction)!=0 || c_instruction=="lw" || c_instruction=="addi"){
				std::string writeregister=pipeline.mem_wb.instruction;
				int registerdata=pipeline.mem_wb.data;
				registers[registerMap[writeregister]]=registerdata;
			}

			//MEM(memory write stage)
			int address1;
			// std::cout<<pipeline.ex_mem.PC<<pipeline.ex_mem.base_instruction<<"\n";
			int data;
			int PC_mem=pipeline.ex_mem.PC;
			int swdata;
			std::string b_instruction=pipeline.ex_mem.base_instruction;
			int Zero=pipeline.ex_mem.zero;
			std::string mem_instruction=pipeline.ex_mem.instruction;
			if(R_type.count(b_instruction)!=0 || b_instruction=="addi"){
				data=pipeline.ex_mem.ALU_result;
			}
			else if(b_instruction=="lw"){
				address1=pipeline.ex_mem.ALU_result;
				// std::cout<<address<<"\n";
				data=memoryDelta[address1/4];
				memoryDelta.erase(address1/4);
				// std::cout<<data<<"\n";
			}
			else if(b_instruction=="sw"){
				address1=pipeline.ex_mem.ALU_result;
				swdata=pipeline.ex_mem.w_data;
				memoryDelta[address1/4]=swdata;
				memoryDelta1[address1/4]=swdata;
			}


			//EX(Execution ALU)
			// std::cout<<pipeline.id_ex.PC<<pipeline.id_ex.instruction<<"\n";
			int PC1=pipeline.id_ex.PC;
			std::string PC_ex;
			int rdata1=pipeline.id_ex.r_data1;
			int rdata2=pipeline.id_ex.r_data2;
			int Alures;
			int writedata;
			std::string base=pipeline.id_ex.instruction;
			std::string ex_instruction;
			if(pipeline.id_ex.instruction=="add"){
				Alures=rdata1+rdata2;
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="sub"){
				Alures=rdata1-rdata2;
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="and"){
				Alures=rdata1 & rdata2;
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="slt"){
				if(rdata1<rdata2){
					Alures=1;
				}
				else{
					Alures=0;
				}
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="or"){
				Alures=rdata1 | rdata2;
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="mul"){
				Alures=rdata1*rdata2;
				// std::cout<<"mul:"<<Alures<<"\n";
				ex_instruction=pipeline.id_ex.instruction15_11;
			}
			else if(pipeline.id_ex.instruction=="addi"){
				Alures=rdata1+stoi(pipeline.id_ex.instruction15_0);
				ex_instruction=pipeline.id_ex.instruction20_16;
			}
			else if(pipeline.id_ex.instruction=="sw"){
				Alures=rdata1+stoi(pipeline.id_ex.instruction15_0);
				writedata=rdata2;
			}
			else if(pipeline.id_ex.instruction=="lw"){
				// std::cout<<"1111"<<"\n";
				// std::cout<<pipeline.id_ex.instruction15_0<<"\n";
				Alures=rdata1+stoi(pipeline.id_ex.instruction15_0);
				ex_instruction=pipeline.id_ex.instruction20_16;
				if(b_instruction=="lw"){
					pipeline.ex_mem.lw_bypass=data;
				}
			}
			else if(pipeline.id_ex.instruction=="beq" || pipeline.id_ex.instruction=="bne"){
				Alures=rdata1-rdata2;
				PC_ex=(pipeline.id_ex.instruction15_0);
				if(base=="beq"){
					if(Alures==0){
						// std::cout<<"pp"<<"\n";
						pipeline.pipelinecontrol.PCsrc=1;
					
					}
				}
				else if(base=="bne"){
					if(Alures=0){
						pipeline.pipelinecontrol.PCsrc=1;
					}
				}


			}


			// ID{instruction decoder}
			// std::cout<<pipeline.if_id.PC<<pipeline.if_id.instruction[0]<<"\n";
			int PC_id=pipeline.if_id.PC;
			std::vector<std::string>id_instruction=pipeline.if_id.instruction;
			std::string read_register1;
			std::string read_register2;
			int read_data1;
			int read_data2;
			int jump=0;
			if(id_instruction[0]=="j"){
				jump=1;
			}
			if(R_type.count(id_instruction[0])!=0){
				read_register1=id_instruction[2];
				read_register2=id_instruction[3];
			}
			else if(I_type.count(id_instruction[0])!=0){
				read_register1=id_instruction[2];
				read_register2=id_instruction[1];
			}
			read_data1=registers[registerMap[read_register1]];
			read_data2=registers[registerMap[read_register2]];
			int stall=0;
			if(base=="lw"){
			if((read_register1==ex_instruction && ex_instruction!="$0" && ex_instruction!="")){
				id_instruction={"add","$0","$0","$0"};
				stall=1;
				read_data1=0;
				read_data2=0;
			}
			if((R_type.count(id_instruction[0])!=0 || id_instruction[0]=="sw" || id_instruction[0]=="beq" || id_instruction[0]=="bne") && read_register2==ex_instruction && ex_instruction!="$0" && ex_instruction!=""){
				id_instruction={"add","$0","$0","$0"};
				stall=1;
				read_data1=0;
				read_data2=0;
			}
			}
			else{
			if((read_register1==ex_instruction && ex_instruction!="$0" && ex_instruction!="")){
				read_data1=Alures;
			}
			else if(read_register1==mem_instruction && mem_instruction!="$0" && mem_instruction!="") {
				read_data1=pipeline.ex_mem.ALU_result;
			}
			if((R_type.count(id_instruction[0])!=0 || id_instruction[0]=="sw" || id_instruction[0]=="beq" || id_instruction[0]=="bne") && read_register2==ex_instruction && ex_instruction!="$0" && ex_instruction!=""){
				read_data2=Alures;
			}
			else if((R_type.count(id_instruction[0])!=0 || id_instruction[0]=="sw" || id_instruction[0]=="beq" || id_instruction[0]=="bne") && read_register2==mem_instruction && mem_instruction!="$0" && mem_instruction!=""){
				read_data2=pipeline.ex_mem.ALU_result;
			}
			}
			if(stall==0 && b_instruction=="lw"){
				if(read_register1==mem_instruction && mem_instruction!="$0" && mem_instruction!="") {
				std::cout<<"data1"<<data<<"\n";
				read_data1=data;
			}
				if((R_type.count(id_instruction[0])!=0 || id_instruction[0]=="sw" || id_instruction[0]=="beq" || id_instruction[0]=="bne") && read_register2==mem_instruction && mem_instruction!="$0" && mem_instruction!=""){
				std::cout<<"data2"<<data<<"\n";
				read_data2=data;
			}
			}
			// std::cout<<"datahazard::"<<datahazard<<"\n";
			

			//IF(instruction fetch)
			// std::cout<< PCcurr <<"\n";
			int PC_if=PCcurr;
			int branch=0;
			std::vector<std::string>if_instruction;
			if(PC_if>=commands.size() || id_instruction[0]=="beq" || id_instruction[0]=="bne") {
				if_instruction={"add","$0","$0","$0"};
				branch=1;
			}
			else if(base=="beq" || base=="bne"){
				if_instruction={"add","$0","$0","$0"};
				branch=2;
			}
			else if(id_instruction[0]=="j"){
				if_instruction={"add","$0","$0","$0"};
			}
			else{
				if_instruction = commands[PC_if];
			}
			input=input+1;


			//SECOND HALF

			//MEM(memory stage)
			pipeline.mem_wb.data=data;
			pipeline.mem_wb.instruction=mem_instruction;
			pipeline.mem_wb.PC=PC_mem;
			pipeline.mem_wb.ba_instruction=b_instruction;
			//EX(Execution stage)
			if(Alures==0){
				pipeline.ex_mem.zero=1;
			}
			pipeline.ex_mem.ALU_result=Alures;
			pipeline.ex_mem.instruction=ex_instruction;
			pipeline.ex_mem.PC_branch=PC_ex;
			pipeline.ex_mem.PC=PC1;
			pipeline.ex_mem.w_data=writedata;
			pipeline.ex_mem.base_instruction=base;


			//ID(instruction decode)
			pipeline.id_ex.PC=PC_id;
			pipeline.id_ex.r_data1=read_data1;
			pipeline.id_ex.r_data2=read_data2;
			pipeline.id_ex.instruction=id_instruction[0];
			if(I_type.count(id_instruction[0])!=0){
				pipeline.id_ex.instruction15_0=id_instruction[3];
				pipeline.id_ex.instruction20_16=read_register2;
			}
			else if(R_type.count(id_instruction[0])!=0){
				pipeline.id_ex.instruction15_11=id_instruction[1];
			}


			//IF(instruction fetch)
			if(stall==0){
			pipeline.if_id.instruction=if_instruction;
			pipeline.if_id.PC=PC_if;
			if (instructions.find(pipeline.if_id.instruction[0]) == instructions.end())
				{
					handleExit(SYNTAX_ERROR, clockCycles);
					return;
				}
			}

		// making PCcurr value..
			if(jump==1){
				PCcurr=address[id_instruction[1]];
			}
			else if(branch==1 || stall==1){
				// std::cout<<"Print:;"<<PCcurr<<"\n";
				PCcurr=PCcurr;
			}
			else if(branch==2 && pipeline.pipelinecontrol.PCsrc==0){
				PCcurr=PCcurr;
			}
			else if(pipeline.pipelinecontrol.PCsrc==1){
				// std::cout<<"branch: "<<PC_ex<<"\n";
				PCcurr=address[PC_ex];
				pipeline.pipelinecontrol.PCsrc=0;
				if(PCcurr>=commands.size()){
					pipeline.pipelinecontrol.Branch=1;
				}if(c_instruction=="beq"||c_instruction=="bne"){
				if(pipeline.pipelinecontrol.Branch==1){
					break;
				}
			}
			}
			else{
				PCcurr++;
			}
			if(c_instruction=="beq"||c_instruction=="bne"){
				if(pipeline.pipelinecontrol.Branch==1){
					break;
				}
			}
		}
	printRegistersAndMemoryDelta(clockCycles);
	// std::cout<<"Total Clockcycles:"<<clockCycles<<"\n";

	}



//79stage pipeline starts from here;;;

	void execute79stagepipeline(){
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		} 
		int clockCycles = 0;
		struct _79stagepipeline pipeline;
		while(pipeline.last.PC+1 < commands.size()){
			printRegistersAndMemoryDelta(clockCycles);
			clockCycles++;
			if(clockCycles==12){
				break;
			}
			//RW2
			std::cout<<pipeline.dm2_rw2.PC<<"\n";
			if(pipeline.dm1_dm2.base_instruction=="lw"){
				// std::cout<<address<<"\n";
				registers[registerMap[pipeline.dm2_rw2.lw_register]]=pipeline.dm2_rw2.lw_data;
				// std::cout<<data<<"\n";
			}
			pipeline.last.PC=pipeline.dm2_rw2.PC;
			std::cout<<"last2"<<pipeline.last.PC;

			//DM2
			std::cout<<pipeline.dm1_dm2.PC<<"\n";
			std::cout<<pipeline.dm1_dm2.base_instruction;
			if(pipeline.dm1_dm2.base_instruction=="lw"){
				
				// std::cout<<address<<"\n";
				pipeline.dm2_rw2.lw_data=memoryDelta[pipeline.dm1_dm2.address];
				memoryDelta.erase(pipeline.dm1_dm2.address);
				pipeline.dm2_rw2.lw_register=pipeline.dm1_dm2.rs_register;
				// std::cout<<data<<"\n";
			}
			else if(pipeline.dm1_dm2.base_instruction=="sw"){
				memoryDelta[pipeline.dm1_dm2.address]=pipeline.dm1_dm2.sw_data;
				memoryDelta1[pipeline.dm1_dm2.address]=pipeline.dm1_dm2.sw_data;
			}
			pipeline.dm2_rw2.base_instruction=pipeline.dm1_dm2.base_instruction;
			pipeline.dm2_rw2.PC=pipeline.dm1_dm2.PC;
			//DM1
			
			pipeline.dm1_dm2.address=pipeline.ex2_dm1.address;
			pipeline.dm1_dm2.PC=pipeline.ex2_dm1.PC;
			pipeline.dm1_dm2.rs_register=pipeline.ex2_dm1.rs_register;
			pipeline.dm1_dm2.base_instruction=pipeline.ex2_dm1.base_instruction;
			pipeline.dm1_dm2.sw_data=pipeline.ex2_dm1.sw_data;

			//EX2
			int PC2=pipeline.rr_ex2.PC;
			std::string PC_ex2;
			int rdata11=pipeline.rr_ex2.r_data1;
			int rdata22=pipeline.rr_ex2.r_data2;
			int address1;
			std::string ex2_instruction;
			int writedata;
			
			if(pipeline.rr_ex2.instruction=="sw"){
				address1=rdata11+stoi(pipeline.rr_ex2.instruction15_0);
				writedata=rdata22;
			}
			else if(pipeline.rr_ex2.instruction=="lw"){
				// std::cout<<"1111"<<"\n";
				// std::cout<<pipeline.id_ex.instruction15_0<<"\n";
				address1=rdata11+stoi(pipeline.rr_ex2.instruction15_0);
				ex2_instruction=pipeline.rr_ex2.instruction20_16;
			}
			pipeline.ex2_dm1.address=address1;
			pipeline.ex2_dm1.PC=PC2;
			pipeline.ex2_dm1.sw_data=writedata;
			pipeline.ex2_dm1.rs_register=ex2_instruction;
			pipeline.ex2_dm1.base_instruction=pipeline.rr_ex2.instruction;

			//RW1
			std::string c_instruction=pipeline.ex1_rw1.base_instruction;
			std::string writeregister;
			// std::cout<<pipeline.mem_wb.PC<<c_instruction<<"\n";
			if(R_type.count(c_instruction)!=0 || c_instruction=="addi"){
				writeregister=pipeline.ex1_rw1.rd_register;
				int registerdata=pipeline.ex1_rw1.ALUresut;
				registers[registerMap[writeregister]]=registerdata;
			}
			if(c_instruction=="add" && writeregister=="$0"){
				pipeline.last.PC=pipeline.last.PC;
			}
			else{
				pipeline.last.PC=pipeline.ex1_rw1.PC;
			}
			std::cout<<"last1"<<pipeline.last.PC;

			//EX1
			int PC1=pipeline.rr_ex1.PC;
			std::string PC_ex1;
			int rdata1=pipeline.rr_ex1.r_data1;
			int rdata2=pipeline.rr_ex1.r_data2;
			int Alures;
			std::string ex1_instruction;
			if(pipeline.rr_ex1.instruction=="add"){
				Alures=rdata1+rdata2;
				ex1_instruction=pipeline.rr_ex1.instruction15_11;
			}
			else if(pipeline.rr_ex1.instruction=="sub"){
				Alures=rdata1-rdata2;
				ex1_instruction=pipeline.rr_ex1.instruction15_11;
			}
			else if(pipeline.rr_ex1.instruction=="and"){
				Alures=rdata1 & rdata2;
				ex1_instruction=pipeline.rr_ex1.instruction15_11;
			}
			else if(pipeline.rr_ex1.instruction=="slt"){
				if(rdata1<rdata2){
					Alures=1;
				}
				else{
					Alures=0;
				}
				ex1_instruction=pipeline.rr_ex1.instruction15_11;
			}
			else if(pipeline.rr_ex1.instruction=="or"){
				Alures=rdata1 | rdata2;
				ex1_instruction=pipeline.rr_ex1.instruction15_11;
			}
			else if(pipeline.rr_ex1.instruction=="mul"){
				Alures=rdata1*rdata2;
				// std::cout<<"mul:"<<Alures<<"\n";
				ex1_instruction=pipeline.rr_ex1.instruction15_11;
			}
			else if(pipeline.rr_ex1.instruction=="addi"){
				Alures=rdata1+stoi(pipeline.rr_ex1.instruction15_0);
				ex1_instruction=pipeline.rr_ex1.instruction20_16;
			}
			else if(pipeline.rr_ex1.instruction=="beq" || pipeline.rr_ex1.instruction=="bne"){
				Alures=rdata1-rdata2;
				PC_ex1=(pipeline.rr_ex1.instruction15_0);
				if(pipeline.rr_ex1.instruction=="beq"){
					if(Alures==0){
						// std::cout<<"pp"<<"\n";
						pipeline.pipelinecontrol.PCsrc=1;
					
					}
				}
				else if(pipeline.rr_ex1.instruction=="bne"){
					if(Alures=0){
						pipeline.pipelinecontrol.PCsrc=1;
					}
				}


			}

			pipeline.ex1_rw1.ALUresut=Alures;
			pipeline.ex1_rw1.base_instruction=pipeline.rr_ex1.instruction;
			pipeline.ex1_rw1.PC=PC1;
			pipeline.ex1_rw1.PC_branch=PC_ex1;
			pipeline.ex1_rw1.rd_register=ex1_instruction;


			//RR
			std::string b_instruction=pipeline.dec2_rr.instruction;
			if(b_instruction=="lw" || b_instruction=="sw"){
				
				pipeline.rr_ex2.PC=pipeline.dec2_rr.PC;
				pipeline.rr_ex2.r_data1=pipeline.dec2_rr.r_data1;
				pipeline.rr_ex2.r_data2=pipeline.dec2_rr.r_data2;
				pipeline.rr_ex2.instruction15_0=pipeline.dec2_rr.instruction15_0;
				pipeline.rr_ex2.instruction20_16=pipeline.dec2_rr.instruction20_16;
				pipeline.rr_ex2.instruction=pipeline.dec2_rr.instruction;
			}
			else if(R_type.count(b_instruction)!=0 || I_type.count(b_instruction)!=0 || J_type.count(b_instruction)!=0){
				pipeline.rr_ex1.PC=pipeline.dec2_rr.PC;
				pipeline.rr_ex1.r_data1=pipeline.dec2_rr.r_data1;
				pipeline.rr_ex1.r_data2=pipeline.dec2_rr.r_data2;
				pipeline.rr_ex1.instruction15_11=pipeline.dec2_rr.instruction15_11;
				pipeline.rr_ex1.instruction20_16=pipeline.dec2_rr.instruction20_16;
				pipeline.rr_ex1.instruction15_0=pipeline.dec2_rr.instruction15_0;
				pipeline.rr_ex1.instruction=pipeline.dec2_rr.instruction;
			}
			//DEC2
			pipeline.dec2_rr.PC=pipeline.dec1_dec2.PC;
			std::vector<std::string>dec_instruction={"","","",""};
			dec_instruction=pipeline.dec1_dec2.instruction;
			std::string read_register1;
			std::string read_register2;
			int read_data1;
			int read_data2;
			int jump=0;
			if(dec_instruction[0]=="j"){
				jump=1;
			}
			std::cout<<"111"<<"\n";
			if(R_type.count(dec_instruction[0])!=0){
				read_register1=dec_instruction[2];
				read_register2=dec_instruction[3];
			}
			else if(I_type.count(dec_instruction[0])!=0){
				read_register1=dec_instruction[2];
				read_register2=dec_instruction[1];
			}
			read_data1=registers[registerMap[read_register1]];
			read_data2=registers[registerMap[read_register2]];


			pipeline.dec2_rr.r_data1=read_data1;
			pipeline.dec2_rr.r_data2=read_data2;
			pipeline.dec2_rr.instruction=dec_instruction[0];
			if(I_type.count(dec_instruction[0])!=0){
				pipeline.dec2_rr.instruction15_0=dec_instruction[3];
				pipeline.dec2_rr.instruction20_16=read_register2;
			}
			else if(R_type.count(dec_instruction[0])!=0){
				pipeline.dec2_rr.instruction15_11=dec_instruction[1];
			}



			//DEC1
			std::cout<<pipeline.if2_dec1.PC<<"\n";
			pipeline.dec1_dec2.PC=pipeline.if2_dec1.PC;
			pipeline.dec1_dec2.instruction=pipeline.if2_dec1.instruction;


			//IF2
			std::cout<<pipeline.if1_if2.PC<<"\n";
			pipeline.if2_dec1.PC=pipeline.if1_if2.PC;
			pipeline.if2_dec1.instruction=pipeline.if1_if2.instruction;


			//IF1
			int PC_if1=PCcurr;
			std::cout<<PCcurr<<"\n";
			int branch=0;
			std::vector<std::string>if_instruction;
			int case1=0;
			if(PC_if1>=commands.size()) {
				case1=1;
			}
			else{
				if_instruction = commands[PC_if1];
			}
			pipeline.if1_if2.PC=PC_if1;
			pipeline.if1_if2.instruction=if_instruction;

			if(pipeline.pipelinecontrol.PCsrc==1){
				pipeline.pipelinecontrol.PCsrc=0;
				std::string labell=pipeline.ex1_rw1.PC_branch;
				PCcurr=address[labell];
			}
			if(case1==1){
				PCcurr==PCcurr;
			}
			else{
				PCcurr++;
			}

		}
		printRegistersAndMemoryDelta(clockCycles);
	}

};
#endif
	// print the register data in hexadecimal