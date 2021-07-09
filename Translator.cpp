#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
using namespace std;
unordered_map<string, double> nums;
double R;
ifstream in;
vector<streampos> line_pos;
int label = 0;
int max_label = 0; //目前到达的最大行号
void ADD(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R += stoi(c);
	}
	else {
		R += nums[c];
	}
}
void SUB(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R -= stoi(c);
	}
	else {
		R -= nums[c];
	}
}
void MUL(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R *= stoi(c);
	}
	else {
		R *= nums[c];
	}
}
void DIV(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R /= stoi(c);
	}
	else {
		R /= nums[c];
	}
}
void MOD(const string& b, const string& c) {
	int tmp = R;
	if (isdigit(c[0])) {
		tmp %= stoi(c);
	}
	else {
		tmp %= static_cast<int>(nums[c]);
	}
	R = tmp;
}
void LD(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = stoi(c);
	}
	else {
		R = nums[c];
	}
}
void ST(const string& b, const string& c) { nums[c] = R; }

void LT(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = R < stoi(c);
	}
	else {
		R = R < nums[c];
	}
}

void GT(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = R > stoi(c);
	}
	else {
		R = R > nums[c];
	}
}

void EQ(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = R == stoi(c);
	}
	else {
		R = R == nums[c];
	}
}

void LE(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = R <= nums[c];
	}
	else {
		R = R <= nums[c];
	}
}

void GE(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = R >= nums[c];
	}
	else {
		R = R >= nums[c];
	}
}

void NE(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = R != static_cast<bool>(nums[c]);
	}
	else {
		R = R != static_cast<bool>(nums[c]);
	}
}

void AND(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = R && static_cast<bool>(nums[c]);
	}
	else {
		R = R && static_cast<bool>(nums[c]);
	}
}

void OR(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = R || static_cast<bool>(nums[c]);
	}
	else {
		R = R || static_cast<bool>(nums[c]);
	}
}

void NO(const string& b, const string& c) {
	if (isdigit(c[0])) {
		R = !nums[c];
	}
	else {
		R = !nums[c];
	}
}
void jump_helper(int jump_label) {
	//如果需要跳转的指令大于当前已经扫描过的语句，则需要提前扫描
	if (jump_label > max_label) {
		streampos cur = in.tellg();
		string tmp;
		int cur_label = label;
		//当前行号已经读了，加入当前行号在第i行，但读头从第i+1行开始
		while (cur_label < max_label) {
			getline(in, tmp);
			++cur_label;
		}
		//读头从第max_label + 1行开始
		while (getline(in,tmp) && cur_label < jump_label) {
			line_pos.push_back(in.tellg());
			++cur_label;
		}
		max_label = jump_label;
		in.seekg(cur);
	}
}

void JMP(const string& b, const string& c) {
	int new_label = stoi(c);
	jump_helper(new_label--);
	//-1是因为从0开始
	in.seekg(line_pos[new_label]);
	//-1是因为后期+1
	label = new_label;
}

void TJ(const string& b, const string& c) {
	int new_label = stoi(c);
	jump_helper(new_label--);
	if (static_cast<bool>(R)) {
		//-1是因为从0开始
		in.seekg(line_pos[new_label]);
		//-1是因为后期+1
		label = new_label;
	}
}

void FJ(const string& b, const string& c) {
	int new_label = stoi(c);
	jump_helper(new_label--);
	if (!static_cast<bool>(R)) {
		//-1是因为从0开始
		in.seekg(line_pos[new_label]);
		//-1是因为后期+1
		label = new_label;
	}
}

unordered_map<string, function<void(const string&)>> print{
	{"INT",
	 [](const string& c) {
		 if (c == "[R]") {
			 cout << static_cast<int>(R);
		 }
 else {
  cout << static_cast<int>(nums[c]);
}
}},
{"BOOL",
 [](const string& c) {
	 if (c == "[R]") {
		 cout << (R != 0);
	 }
else {
 cout << static_cast<int>(nums[c]);
}
}},
{"FLOAT",
 [](const string& c) {
	 if (c == "[R]") {
		 cout << R;
	 }
else {
 cout << static_cast<int>(nums[c]);
}
}},
{"CHAR", [](const string& c) {
	 if (c == "[R]") {
		 cout << static_cast<char>(R);
	 }
else if (isdigit(c[0])) {
 cout << static_cast<char>(stoi(c));
}
else if (c[0] == '\'') {
 cout << c[1];
}
else {
 cout << static_cast<int>(nums[c]);
}
}} };

void PRINT(const string& b, const string& c) {
	if (b == "_") {
		cout << c;
	}
	else {
		if (print.count(b) == 0) {
			cout << "\nLine:" << label << " Unknown args : " << b << endl;
		}
		else {
			print[b](c);
		}
	}
}

unordered_map<string, function<void(const string&, const string&)>> command{
	{"ADD", ADD}, {"SUB", SUB}, {"MUL", MUL}, {"DIV", DIV}, {"MOD", MOD},
	{"LD", LD},   {"ST", ST},   {"LT", LT},   {"GT", GT},   {"EQ", EQ},
	{"LE", LE},   {"GE", GE},   {"NE", NE},   {"AND", AND}, {"OR", OR},
	{"NO", NO},   {"JMP", JMP}, {"FJ", FJ},   {"TJ", TJ},   {"PRINT", PRINT} };


int main() {
	R = 0;
	in.open("assembly.txt");
	streampos pos;
	string line;
	string a, b, c, tmp;
	stringstream ss;
	//第一行指令开头
	line_pos.emplace_back(in.tellg());
	while (getline(in, line)) {
		++label;
		if (label > max_label) {
			max_label = label;
			//当前指令结尾，即下一条指令开头
			line_pos.emplace_back(in.tellg());
		}
		ss.clear();
		ss.str(line);
		//ss >> a >> b >> c;
		//PRINT中c可能包含空格
		ss >> a >> b;
		c.clear();
		while (ss >> tmp) {
			c.append(tmp);
			c.push_back(' ');
		}
		c.pop_back();
		if (command.count(a) == 0) {
			cout << "\nLine:" << label << " Unknown command : " << a << endl;
			break;
		}
		command[a](b, c);
	}
	return 0;
}