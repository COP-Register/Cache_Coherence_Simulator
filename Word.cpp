#include "Word.h"

Word::Word(const int new_offset)
    : offset(new_offset)
{
}

std::shared_ptr<Word> Word::create(const int new_offset)
{
    return std::shared_ptr<Word>(new Word(new_offset));
}

void Word::set_offset(const int new_offset)
{
    offset = new_offset;
}

int Word::get_offset() const
{
    return offset;
}
