#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <bitset>
// 이런거 헤더 내맘대로 써도 되는건가?

// R, I, J 타입 명령어 저장
const std::unordered_map<std::string, char> INSTRUCTION_TYPE =
    {
        // R-type
        {"add", 'R'},
        {"and", 'R'},
        {"jr", 'R'},
        {"nor", 'R'},
        {"or", 'R'},
        {"slt", 'R'},
        {"sll", 'R'},
        {"srl", 'R'},
        {"sub", 'R'},

        // I-type
        {"addi", 'I'},
        {"andi", 'I'},
        {"beq", 'I'},
        {"bne", 'I'},
        {"lui", 'I'},
        {"lw", 'I'},
        {"ori", 'I'},
        {"slti", 'I'},
        {"sw", 'I'},

        // J-type
        {"j", 'J'},
        {"jal", 'J'}};
// R 타입 funct값 저장
const std::unordered_map<std::string, int> R_CODE =
    {
        {"add", 0x20},
        {"and", 0x24},
        {"jr", 0x08},
        {"nor", 0x27},
        {"or", 0x25},
        {"slt", 0x2a},
        {"sll", 0x00},
        {"srl", 0x02},
        {"sub", 0x22}};
// I 타입 opcode값 저장
const std::unordered_map<std::string, int> I_CODE =
    {
        {"addi", 0x08},
        {"andi", 0x0c},
        {"beq", 0x04},
        {"bne", 0x05},
        {"lui", 0x0f},
        {"lw", 0x23},
        {"ori", 0x0d},
        {"slti", 0x0a},
        {"sw", 0x2b}};
// J 타입 opcode값 저장
const std::unordered_map<std::string, int> J_CODE =
    {
        {"j", 0x02},
        {"jal", 0x03}};

int R_to_b(std::vector<std::string> &v)
{
    std::string op = v.at(0);
    int opcode, rs, rt, rd, shamt, funct;
    opcode = 0;
    funct = R_CODE.find(op)->second;

    if (op == "jr")
    {
        rs = std::stoi(v.at(1).substr(1), nullptr, 0);
        rt = 0;
        rd = 0;
        shamt = 0;
    }
    else if (op == "sll" || op == "srl")
    {
        rs = 0;
        rt = std::stoi(v.at(2).substr(1), nullptr, 0);
        rd = std::stoi(v.at(1).substr(1), nullptr, 0);
        shamt = std::stoi(v.at(3), nullptr, 0);
    }
    else
    {
        rs = std::stoi(v.at(2).substr(1), nullptr, 0);
        rt = std::stoi(v.at(3).substr(1), nullptr, 0);
        rd = std::stoi(v.at(1).substr(1), nullptr, 0);
        shamt = 0;
    }

    int instruction_binary = 0;
    instruction_binary |= (opcode << 26);
    instruction_binary |= (rs << 21);
    instruction_binary |= (rt << 16);
    instruction_binary |= (rd << 11);
    instruction_binary |= (shamt << 6);
    instruction_binary |= funct;

    return instruction_binary;
}

int I_to_b(std::vector<std::string> &v)
{
    std::string op = v.at(0);
    int opcode, rs, rt, immediate;
    opcode = I_CODE.find(op)->second;

    if (op == "addi" || op == "andi" || op == "ori" || op == "slti")
    {
        rs = std::stoi(v.at(2).substr(1), nullptr, 0);
        rt = std::stoi(v.at(1).substr(1), nullptr, 0);
        immediate = std::stoi(v.at(3), nullptr, 0);
    }
    else if (op == "beq" || op == "bne")
    {
        rs = std::stoi(v.at(1).substr(1), nullptr, 0);
        rt = std::stoi(v.at(2).substr(1), nullptr, 0);
        immediate = std::stoi(v.at(3), nullptr, 0);
    }
    else if (op == "lui")
    {
        rs = 0;
        rt = std::stoi(v.at(1).substr(1), nullptr, 0);
        immediate = std::stoi(v.at(2), nullptr, 0);
    }
    else if (op == "lw" || op == "sw")
    {
        std::string offset_term = v.at(2);
        std::string offset, reg;
        bool in_parenthesis = false;
        for (char c : offset_term)
        {
            if (c == '(')
            {
                in_parenthesis = true;
                continue;
            }
            else if (c == ')')
            {
                break;
            }
            if (!in_parenthesis)
            {
                offset += c;
            }
            else
            {
                reg += c;
            }
        }
        rs = std::stoi(reg.substr(1), nullptr, 0);
        rt = std::stoi(v.at(1).substr(1), nullptr, 0);
        immediate = std::stoi(offset, nullptr, 0);
    }

    int instruction_binary = 0;
    instruction_binary |= (opcode << 26);
    instruction_binary |= (rs << 21);
    instruction_binary |= (rt << 16);
    instruction_binary |= immediate & 0xffff;

    return instruction_binary;
}

int J_to_b(std::vector<std::string> &v)
{
    std::string op = v.at(0);
    int opcode, address;
    opcode = J_CODE.find(op)->second;

    address = std::stoi(v.at(1), nullptr, 0);

    int instruction_binary = 0;
    instruction_binary |= (opcode << 26);
    instruction_binary |= (address >> 2) & 0x03ffffff;

    return instruction_binary;
}

/*
메모리 구조체
데이터, 텍스트를 구분해서 메모리 주소에 맞게 저장
*/
struct memory_struct
{
    static bool instruction_has_label(std::vector<std::string> &inst_v)
    {
        try
        {
            std::stoi(inst_v.back(), nullptr, 0);
            return false;
        }
        catch (const std::invalid_argument &)
        {
            if (inst_v.back()[0] == '$')
            {
                return false;
            }
            return true;
        }
    }

    struct DataBundle
    {
        int address;
        int data_binary;
        std::string label;
        DataBundle(int address, int data_binary, std::string label = "")
            : address(address), data_binary(data_binary), label(label) {}
    };
    struct TextBundle
    {
        int address;
        std::vector<std::string> instruction_line;
        std::string label;
        int text_binary;
        bool has_label = false;
        TextBundle(int address, std::vector<std::string> instruction_line, std::string label = "")
            : address(address), instruction_line(instruction_line), label(label)
        {
            if (instruction_has_label(instruction_line))
            {
                has_label = true;
            }
        }
    };

    int textSegmentAddress = 0x400000;
    int dataSegmentAddress = 0x10000000;
    std::vector<DataBundle> datas;
    std::vector<TextBundle> texts;

    // 바이너리로 바꾼 데이터, 라벨 넣으면 주소 계산해서 datas에 추가하는 함수
    void data_push(int b_data, std::string label)
    {
        DataBundle d_bundle(dataSegmentAddress, b_data, label);
        dataSegmentAddress += 4; // 1 word = 4 byte
        datas.push_back(d_bundle);
    }

    // 데이터 라벨 넣으면 주소 리턴하는 함수
    int data_get_address(std::string label)
    {
        for (const DataBundle &d : datas)
        {
            // 같은 라벨의 다른 데이터가 있을 수 있나? 그렇다면 수정 필요
            // 아니면 array같은건 저장할 때 라벨 없는 데이터에 라벨로 접근하나? 그렇다면 수정 필요
            if (d.label == label)
            {
                return d.address;
            }
        }
        return -1; // data segment에 해당 라벨 데이터가 없으면 리턴 -1
    }

    // parsed instruction vector, 라벨 넣으면 주소 계산해서 texts에 추가하는 함수
    // 인스트럭션에 라벨 포함돼있는지 보고 있으면 has_label true(2번째 iteration 때 사용)
    // pseudo instruction 미리 쪼개서 들어와야 됨
    void text_push(std::vector<std::string> instruction_line, std::string label = "")
    {
        TextBundle t_bundle(textSegmentAddress, instruction_line, label);
        textSegmentAddress += 4;
        texts.push_back(t_bundle);
    }

    // 텍스트 라벨 넣으면 주소 리턴하는 함수
    int text_get_address(std::string label)
    {
        for (const TextBundle &t : texts)
        {
            if (t.label == label)
            {
                return t.address;
            }
        }
        return -1; // text segment에 해당 라벨 데이터가 없으면 리턴 -1
    }

    int get_address(std::string label)
    {
        const int d_get = data_get_address(label);
        const int t_get = text_get_address(label);
        if (d_get != -1)
        {
            return d_get;
        }
        else if (t_get != -1)
        {
            return t_get;
        }
        return -1; // data, text 어디에도 해당 라벨이 없으면 리턴 -1
    }

    // 메모리 texts labels 주소로 변환해서 저장하는 함수
    void texts_labels_to_address()
    {
        for (auto &i : texts)
        {
            if (i.has_label)
            {
                int adr = get_address(i.instruction_line.back());
                if (i.instruction_line.at(0) == "beq" || i.instruction_line.at(0) == "bne") {
                    adr = (adr - (i.address + 4)) / 4;
                }
                i.instruction_line.back() = std::to_string(adr);
                // i.has_label = false;
            }
        }
    }

    void texts_inst_to_binary()
    {
        for (auto &i : texts)
        {
            switch ((INSTRUCTION_TYPE.find(i.instruction_line.at(0))->second))
            {
            case 'R':
                i.text_binary = R_to_b(i.instruction_line);
                break;
            case 'I':
                i.text_binary = I_to_b(i.instruction_line);
                break;
            case 'J':
                i.text_binary = J_to_b(i.instruction_line);
                break;
            }
        }
    }

    std::string memory_binary_export()
    {
        std::string f = "";
        int text_size = texts.size() * 4;
        int data_size = datas.size() * 4;

        std::bitset<32> t_binary(text_size);
        std::bitset<32> d_binary(data_size);
        f.append(t_binary.to_string());
        f.append(d_binary.to_string());

        for (const auto &t : texts)
        {
            std::bitset<32> t_binary(t.text_binary);
            f.append(t_binary.to_string());
        }

        for (const auto &d : datas)
        {
            std::bitset<32> d_binary(d.data_binary);
            f.append(d_binary.to_string());
        }
        return f;
    }
};

// 글로벌 메모리 인스턴스
memory_struct MyMemory;

/*
파서 클래스
파일에서 들어오는 라인을 데이터, 텍스트로 구분해 파싱하고 메모리 구조체에 저장
*/
class Parser
{
private:
    std::string parse_mode;
    std::string data_label_temp;
    std::string text_label_temp;
    std::vector<std::string> preprocessed_line; // 띄어쓰기로 잘라서 콜론, 콤마 지우고 스트링으로 저장한 라인

    int string_to_int(std::string &num_str)
    {
        if (num_str.length() > 2 && (num_str[0] == '0' && (num_str[1] == 'x' || num_str[1] == 'X')))
        {
            return std::stoi(num_str, nullptr, 16);
        }
        else
        {
            return std::stoi(num_str, nullptr, 10);
        }
    }

    // 전처리 해서 preprocessed_line에 저장하고 directive 라인인지 리턴
    std::string preprocess(std::string &l)
    {
        std::istringstream stream(l);
        std::string token;

        preprocessed_line.clear();
        while (stream >> token)
        {
            token.erase(std::remove_if(token.begin(), token.end(), [](char c)
                                       { return c == ',' || c == ':'; }));
            preprocessed_line.push_back(token);
        }
        if (preprocessed_line.at(0) == ".data")
        {
            return ".data";
        }
        else if (preprocessed_line.at(0) == ".text")
        {
            return ".text";
        }
        return "false";
    }

    bool check_exception_pseudo(std::vector<std::string> &t_vector)
    {
        if (t_vector.at(0) == "la")
        {
            int immediate = MyMemory.get_address(t_vector.back());
            int upper_16bit = (immediate >> 16) & 0xffff;
            int lower_16bit = immediate & 0xffff;

            std::vector<std::string> lui_vec = {"lui", t_vector.at(1), std::to_string(upper_16bit)};
            std::vector<std::string> ori_vec = {"ori", t_vector.at(1), t_vector.at(1), std::to_string(lower_16bit)};
            if (lower_16bit == 0)
            {
                parse_text_line(lui_vec);
            }
            else
            {
                parse_text_line(lui_vec);
                parse_text_line(ori_vec);
            }
            return true;
        }
        return false;
    }

    void parse_data_line(std::vector<std::string> d_vector)
    {
        int num_of_element = d_vector.size();
        // decimal or hexadecimal string to int
        int b_data = string_to_int(d_vector.back());

        // 라벨이 있는 데이터면
        if (num_of_element == 3)
        {
            // 메모리 인스턴스는 글로벌로 만들어져 있어야 함
            data_label_temp = d_vector.at(0);
            MyMemory.data_push(b_data, data_label_temp);
            return;

            // 데이터 영역 시작하자마자 라벨없이 데이터만 있는 경우
        }
        else if (num_of_element == 2 && data_label_temp.empty())
        {
            // 라벨에 빈 스트링 전달
            MyMemory.data_push(b_data, data_label_temp);
            return;

            // 어레이 두 번째부터, 아니면 그냥 라벨 없는 데이터 -> 둘다 일단 이전 라벨 붙일거임
        }
        else if (num_of_element == 2 && !data_label_temp.empty())
        {
            MyMemory.data_push(b_data, data_label_temp);
            return;
        }
        else
        {
            std::cout << "Error! parse_data_line" << std::endl;
            return;
        }
    }
    void parse_text_line(std::vector<std::string> t_vector)
    {
        bool is_pseudo = check_exception_pseudo(t_vector);
        if (is_pseudo)
        {
            return;
        }
        int num_of_element = t_vector.size();
        if (num_of_element == 1)
        {
            text_label_temp = t_vector.at(0);
            return;

            // 라벨 바로 다음 인스트럭션 라인
        }
        else if (!text_label_temp.empty())
        {
            MyMemory.text_push(t_vector, text_label_temp);
            text_label_temp = "";
            return;

            // 나머지 인스트럭션 라인, 라벨 없음
        }
        else
        {
            MyMemory.text_push(t_vector);
            return;
        }
    }

public:
    Parser()
    {
        parse_mode = "";
        data_label_temp = "";
        text_label_temp = "";
    }

    void parse_to_mem(std::string l)
    {
        std::string is_directive = preprocess(l);
        if (is_directive == ".data")
        {
            parse_mode = "data";
            return;
        }
        else if (is_directive == ".text")
        {
            parse_mode = "text";
            return;
        }
        else if (is_directive == "false" && parse_mode == "data")
        {
            parse_data_line(preprocessed_line);
            return;
        }
        else if (is_directive == "false" && parse_mode == "text")
        {
            parse_text_line(preprocessed_line);
            return;
        }
        else
        {
            std::cout << "Parsing Error!" << std::endl;
            return;
        }
    }
};

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        std::cerr << "Usage: ./runfile <assembly file>\n";
        std::cerr << "Example) ./runfile ./sample_input/example1.s\n";
        return 0;
    }
    else
    {

        // For input file read (sample_input/example*.s)
        std::string file = argv[1];

        if (freopen(file.c_str(), "r", stdin) == nullptr)
        {
            std::cerr << "File open Error!\n";
            return 1;
        }
        // From now on, if you want to read string from input file, you can just use scanf function.

        // test input file stream redirection to stdin
        std::string line;
        Parser p;
        while (std::getline(std::cin, line))
        {
            p.parse_to_mem(line);
        }

        MyMemory.texts_labels_to_address();
        MyMemory.texts_inst_to_binary();
        std::string final_output = MyMemory.memory_binary_export();

        // For output file write
        // Change .s extension to .o
        file.replace(file.find_last_of('.'), std::string::npos, ".o");
        if (freopen(file.c_str(), "w", stdout) == nullptr)
        {
            std::cerr << "File open Error!\n";
            return 1;
        }

        // If you use std::cout from now on, the result will be written to the output file.

        std::cout << final_output;
    }

    return 0;
}