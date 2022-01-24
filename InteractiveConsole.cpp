//
// Created by user on 14.01.2022.
//

#include "InteractiveConsole.h"
#include <iostream>

TResult ShowMessage(const TString& value, bool isQuestion, bool isInfo)
{
    std::cout << value << std::endl;
    return TResult();
}