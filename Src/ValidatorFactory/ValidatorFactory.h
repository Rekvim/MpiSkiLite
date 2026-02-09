#ifndef VALIDATORFACTORY_H
#define VALIDATORFACTORY_H

#pragma once

#include <QValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QHash>

class ValidatorFactory
{
public:
    enum class Type {
        Digits, // только цифры
        DigitsDot, // цифры с точкой
        NoSpecialChars, // запрет символов
        DigitsHyphens, // цифры и дефис
        LettersHyphens, // буквы и дефис
        FloatNumber, // число с плавающей точкой (±)
        FloatRange, // диапазон "a.b-c.d" (±)
        Percent0to100, // проценты от 0 до 100
        FloatSequence
    };

    static QValidator* create(Type type, QObject* parent = nullptr);

private:
    ValidatorFactory() = delete;
};

#endif // VALIDATORFACTORY_H
