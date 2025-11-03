class Parser {
private:
	std::string parse_mode;
	std::string data_label_temp;
	std::string text_label_temp;
	std::vector<std::string> preprocessed_line; // 띄어쓰기로 잘라서 콜론, 콤마 지우고 스트링으로 저장한 라인
	
    // 전처리 해서 preprocessed_line에 저장하고 directive 라인인지 리턴
	std::string preprocess(std::string &l) {
        std::istringstream stream(l);
        std::string token;

        processed_line.clear();
        while (stream >> token){ 
            token.erase(std::remove_if(token.begin(), token.end(), [](char c) {return c == ',' || c == ':'}));
            preprocessed_line.push_back(token);
        }
        if (preprocessed_line.at(0) == ".data") {
            return ".data";
        } else if (preprocessed_line.at(0) == ".text") {
            return ".text";
        }    
        return "false";
    }

	void parse_data_line(std::vector<std::string> d_vector) {
		int num_of_element = d_vector.size();
        // decimal or hexadecimal string to int
        int b_data;
        std::string &num_str = d_vector.back();
        if (num_str.length() > 2 && (num_str[0] == '0' && (num_str[1] == 'x' || num_str[1] == 'X'))) {
            b_data = std::stoi(num_str, nullptr, 16);
        } else {
            b_data = std::stoi(num_str, nullptr, 10);
        }

        // 라벨이 있는 데이터면
		if (num_of_element == 3) {
			//메모리 인스턴스는 글로벌로 만들어져 있어야 함
			data_label_temp = d_vector.at(0);
			memory_instance.data_push(b_data, data_label_temp);
			return;

		// 데이터 영역 시작하자마자 라벨없이 데이터만 있는 경우
		} else if (num_of_element == 2 && data_label_temp.empty()) {
            // 라벨에 빈 스트링 전달
			memory_instance.data_push(b_data, data_label_temp);
			return;

		// 어레이 두 번째부터, 아니면 그냥 라벨 없는 데이터 -> 둘다 일단 이전 라벨 붙일거임
		} else if (num_of_element == 2 && !data_label_temp.empty()) {
			memory_instance.data_push(b_data, data_label_temp);
			return;

		} else {
			std::cout << "Error! parse_data_line" << std::endl;
			return;
		}
	}
	void parse_text_line(std::vector<std::string> t_vector) {
		int num_of_element = t_vector.size();
        if (num_of_element == 1) {
            text_label_temp = t_vector.at(0);
            return;

        // 라벨 바로 다음 인스트럭션 라인
        } else if (!text_label_temp.empty()) {
            memory_instance.text_push(t_vector, text_label_temp);
            text_label_temp = "";
            return;

        // 나머지 인스트럭션 라인, 라벨 없음
        } else {
            memory_instance.text_push(t_vector);
            return;
        }
	}
public:
	Parser() {
		parse_mode = "";
		data_label_temp = "";
		text_label_temp = "";
	}

	void parse_to_mem(std::string l) {
		std::string is_directive = preprocess(l);
		if (is_directive == ".data") {
			parse_mode = "data";
			return;
		} else if (is_directive == ".text") {
			parse_mode = "text";
			return;
		} else if (is_directive == "false" && parse_mode == "data") {
			parse_data_line(preprocessed_line);
			return;
		} else if (is_directive == "false" && parse_mode == "text") {
			parse_text_line(preprocessed_line);
			return;
		} else {
			std::cout << "Parsing Error!" << std::endl;
			return;
		}
	}
}

Parser p;
std::string line;
while(std::getline(std::cin, line)){
	//parsing 하고 메모리 저장까지
	p.parse_to_mem(line);
}