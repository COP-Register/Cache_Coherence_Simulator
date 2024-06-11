#ifndef WORD_H
#define WORD_H

#include <memory>

class Word
{
public:
    static std::shared_ptr<Word> create(int new_offset);
    void set_offset(int new_offset);
    [[nodiscard]] int get_offset() const;
private:
    int offset;
    explicit Word(int new_offset);
};

#endif //WORD_H
