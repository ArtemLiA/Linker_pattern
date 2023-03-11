#include <iostream>
#include <vector>
#include <memory>
#include <random>

//PASSWORD DEFAULT LENGTH
class password_default_length{
private:
    static inline size_t len = 10;
public:
    static void set_length(size_t new_l){
        if (new_l <= 0){
            throw std::out_of_range("Password default length: invalid length value");
        }
        len = new_l;
    }
    static size_t length(){
        return len;
    }
};

//FUNCTION-GENERATOR OF RANDOM STRING
std::string random_string(const size_t& len, const std::string& chars){
    std::uniform_int_distribution<> distribution(0, chars.length() - 1);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::string rand_str(1, chars[distribution(gen)]);

    for (size_t idx = 0; idx < len - 1; idx++){
        rand_str.push_back(chars[distribution(gen)]);
    }
    return rand_str;
}

class password_generator{
public:
    virtual std::string generate(){
        throw std::runtime_error("password_generator: can't generate.");
    };
    virtual std::string allowed_chars(){
        throw std::runtime_error("password_generator: can't get allowed_chars.");
    };
    virtual size_t length() {
        throw std::runtime_error("password_generator: can't return length.");
    };
    virtual void add(std::shared_ptr<password_generator> ptr){
        throw std::runtime_error("password_generator: can't add.");
    };
};

class basic_password_generator: public password_generator{
public:
    explicit basic_password_generator(size_t l = password_default_length::length()): password_generator(), len(l) {};
    //Generate random string
    std::string generate() final{
        return random_string(len, allowed_chars());
    }
    size_t length() final{
        return len;
    }
private:
    size_t len;
};

class digit_generator: public basic_password_generator{
public:
    explicit digit_generator(size_t l = password_default_length::length()): basic_password_generator(l) {};
    std::string allowed_chars() final{
        return "0123456789";
    }
};

class symbol_generator: public basic_password_generator{
public:
    explicit symbol_generator(size_t l = 12): basic_password_generator(l) {};
    std::string allowed_chars() final{
        return "-/.;#@%)*";
    }
};

class upper_letter_generator: public basic_password_generator{
public:
    explicit upper_letter_generator(size_t l = password_default_length::length()): basic_password_generator(l) {};
    std::string allowed_chars() final{
        return "ABCDEFGHKLMNIOPRST"; //Example
    }
};

class lower_letter_generator: public basic_password_generator{
public:
    explicit lower_letter_generator(size_t l = 12): basic_password_generator(l) {};
    std::string allowed_chars() final{
        return "abcdefghklmnioprst"; //Example
    }
};


//COMPOSITE_PASSWORD_GENERATOR
class composite_password_generator: public password_generator{
private:
    std::vector<std::shared_ptr<password_generator>> generators;
public:
    size_t length() final{
        size_t len = 0;
        auto find_max = [&](const std::shared_ptr<password_generator>& p){if(p->length() > len){len = p->length();}};
        std::for_each(generators.cbegin(), generators.cend(), find_max);
        return len;
    }
    void add(std::shared_ptr<password_generator> ptr) final{
        generators.push_back(ptr);
    }
    std::string generate() final{
        std::vector<std::string> generated_strings;
        std::string concatenated_string;
        std::string password;
        size_t pas_len = length();
        //Finding size
        for (const auto& generator: generators){
            generated_strings.push_back(generator->generate());
        }
        if (pas_len <= generated_strings.size()){
            throw std::out_of_range("composite password generator: too short password");
        }
        for (const auto& str: generated_strings){
            concatenated_string.append(str);
        }
        password = random_string(pas_len - generated_strings.size(), concatenated_string);
        for (const auto& str: generated_strings){
            password.append(random_string(1, str));
        }
        return password;
    }
};

int main() {
    password_default_length::set_length(16);
    std::cout << password_default_length::length() << std::endl;
    std::shared_ptr<password_generator> cond1(new digit_generator()); //length = 16
    std::shared_ptr<password_generator> cond2(new symbol_generator()); //length = 16
    std::shared_ptr<password_generator> cond3(new upper_letter_generator()); //length = 16
    std::shared_ptr<password_generator> cond4(new lower_letter_generator()); //length = 16

    composite_password_generator gen;
    gen.add(cond1);
    gen.add(cond2);
    gen.add(cond3);
    gen.add(cond4);

    std::string password = gen.generate();

    std::cout << "Generated password:" << password << std::endl;
    std::cout << "Size of generated password:" << password.size() << std::endl;

    std::cout << std::endl;
    std::cout << "New password (1):" << gen.generate() << std::endl;
    std::cout << "New password (2):" << gen.generate() << std::endl;
    return 0;
}
