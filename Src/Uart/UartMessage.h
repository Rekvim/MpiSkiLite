#ifndef UARTMESSAGE_H
#define UARTMESSAGE_H

#pragma once
#include <QByteArray>

static constexpr char startbyte = 0xAA;

enum class Command : quint8 {
    DAC = 0xC2, // Установить напряжение ЦАП
    ADC = 0xE1, // Запросить значение АЦП

    SetChADC = 0x98, // Выбрать канал АЦП
    ADC_OnOff = 0x48, // Включить/выключить АЦП
    GetVersion = 0x00, // Запросить версию ПО
    TimerArr = 0x73, // Настроить таймер

    SetDO = 0x41, // Установить цифровой выход
    GetDO = 0x42, // Прочитать цифровые выходы
    GetDI = 0x43, // Прочитать цифровые входы

    OK = 0x25, // Успешное выполнение команды
    WrongCRC = 0x66, // Ошибка контрольной суммы
    UnknownCode = 0x13  // Неизвестная команда
};

class UartMessage
{
public:
    UartMessage() = default;
    UartMessage(const Command command);
    UartMessage(const Command command, const QByteArray& data);
    UartMessage(const Command command, quint16 data);
    UartMessage(const Command command, quint8 data);
    UartMessage(const QByteArray& data);

    QByteArray toByteArray() const;
    bool checkCrc() const;
    void updateCrc();
    Command command() const;
    QByteArray data() const;

private:
    QByteArray crc16() const;
    Command m_command { Command::UnknownCode };
    QByteArray m_data;
    QByteArray m_crc;
};

#endif // UARTMESSAGE_H
