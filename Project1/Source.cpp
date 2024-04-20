#include <windows.h>
#include <string>
#include <stack>
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include "resource.h"

#define ID_BUTTON_0 1000
#define ID_BUTTON_1 1001
#define ID_BUTTON_2 1002
#define ID_BUTTON_3 1003
#define ID_BUTTON_4 1004
#define ID_BUTTON_5 1005
#define ID_BUTTON_6 1006
#define ID_BUTTON_7 1007
#define ID_BUTTON_8 1008
#define ID_BUTTON_9 1009
#define ID_BUTTON_ADD 1010
#define ID_BUTTON_SUBTRACT 1011
#define ID_BUTTON_MULTIPLY 1012
#define ID_BUTTON_DIVIDE 1013
#define ID_BUTTON_EQUALS 1014
#define ID_BUTTON_CLEAR 1015
#define ID_EDIT_RESULT 1016
#define ID_BUTTON_DOT 1017
#define ID_BRAKET_LEFT 1018
#define ID_BRAKET_RIGHT 1019
#define ID_BUTTON_DEL 1020
#define IDM_INFO 1
#define IDM_ABOUT 2
#define IDM_EXIT 3

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddButton(HWND hwnd, LPCWSTR text, int x, int y, int width, int height, int id);
void AppendTextToEdit(HWND hwnd, LPCWSTR text);
void PerformOperation(HWND hwnd, WCHAR operation);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"CalculatorClass";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        L"CalculatorClass",
        L"Калькулятор",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 265, 430,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, IDM_INFO, L"Информация");
    AppendMenu(hSubMenu, MF_STRING, IDM_ABOUT, L"Разработчик");
    AppendMenu(hSubMenu, MF_STRING, IDM_EXIT, L"Выход");
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, L"Меню");
    SetMenu(hwnd, hMenu); 

    RegisterHotKey(hwnd, 1, 0, VK_RETURN);
    RegisterHotKey(hwnd, 2, 0, VK_BACK);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

bool isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/');
}

double performOperation(double operand1, double operand2, char op) {
    switch (op) {
    case '+':
        return operand1 + operand2;
    case '-':
        return operand1 - operand2;
    case '*':
        return operand1 * operand2;
    case '/':
        if (operand2 != 0)
            return operand1 / operand2;
        else
            throw std::invalid_argument("Деление на ноль");
    default:
        throw std::invalid_argument("Некорректный оператор"");
    }
}

double calculate_expression(const std::wstring& expression) {
    std::stack<double> operands;
    std::stack<char> operators;

    for (size_t i = 0; i < expression.length(); ++i) {
        char token = expression[i];

        if (isdigit(token)) {
            size_t j = i;
            while (j < expression.length() && (isdigit(expression[j]) || expression[j] == '.')) {
                ++j;
            }
            std::wstring number_str = expression.substr(i, j - i);
            double number = std::stod(number_str);
            operands.push(number);
            i = j - 1;
        }
        else if (isOperator(token)) {
            while (!operators.empty() && operators.top() != '(' &&
                ((token != '*' && token != '/') || (operators.top() == '*' || operators.top() == '/'))) {
                char op = operators.top();
                operators.pop();
                if (operands.size() < 2) {
                    throw std::invalid_argument("Недостаточно операндов");
                }
                double operand2 = operands.top();
                operands.pop();
                double operand1 = operands.top();
                operands.pop();
                double result = performOperation(operand1, operand2, op);
                operands.push(result);
            }
            operators.push(token);
        }
        else if (token == '(') {
            operators.push(token);
        }
        else if (token == ')') {
            while (!operators.empty() && operators.top() != '(') {
                char op = operators.top();
                operators.pop();
                if (operands.size() < 2) {
                    throw std::invalid_argument("Недостаточно операндов");
                }
                double operand2 = operands.top();
                operands.pop();
                double operand1 = operands.top();
                operands.pop();
                double result = performOperation(operand1, operand2, op);
                operands.push(result);
            }
            if (operators.empty()) {
                throw std::invalid_argument("Несогласованные скобки");
            }
            operators.pop();
        }
    }

    while (!operators.empty()) {
        char op = operators.top();
        operators.pop();
        if (operands.size() < 2) {
            throw std::invalid_argument("Недостаточно операндов");
        }
        double operand2 = operands.top();
        operands.pop();
        double operand1 = operands.top();
        operands.pop();
        double result = performOperation(operand1, operand2, op);
        operands.push(result);
    }

    if (operands.size() != 1) {
        throw std::invalid_argument("Некорректное выражение");
    }

    return operands.top();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static std::wstring currentNumber;
    static std::wstring firstNumber;
    static WCHAR lastOperation = L' ';
    static bool hasOperator = false;
    static bool isResultDisplayed = false;

    switch (uMsg) {
    case WM_CREATE:
        AddButton(hwnd, L"0", 70, 310, 50, 50, ID_BUTTON_0);
        AddButton(hwnd, L"1", 10, 250, 50, 50, ID_BUTTON_1);
        AddButton(hwnd, L"2", 70, 250, 50, 50, ID_BUTTON_2);
        AddButton(hwnd, L"3", 130, 250, 50, 50, ID_BUTTON_3);
        AddButton(hwnd, L"4", 10, 190, 50, 50, ID_BUTTON_4);
        AddButton(hwnd, L"5", 70, 190, 50, 50, ID_BUTTON_5);
        AddButton(hwnd, L"6", 130, 190, 50, 50, ID_BUTTON_6);
        AddButton(hwnd, L"7", 10, 130, 50, 50, ID_BUTTON_7);
        AddButton(hwnd, L"8", 70, 130, 50, 50, ID_BUTTON_8);
        AddButton(hwnd, L"9", 130, 130, 50, 50, ID_BUTTON_9);
        AddButton(hwnd, L"+", 190, 190, 50, 50, ID_BUTTON_ADD);
        AddButton(hwnd, L"-", 190, 130, 50, 50, ID_BUTTON_SUBTRACT);
        AddButton(hwnd, L"*", 130, 70, 50, 50, ID_BUTTON_MULTIPLY);
        AddButton(hwnd, L"/", 190, 250, 50, 50, ID_BUTTON_DIVIDE);
        AddButton(hwnd, L"=", 190, 310, 50, 50, ID_BUTTON_EQUALS);
        AddButton(hwnd, L"C", 130, 310, 50, 50, ID_BUTTON_CLEAR);
        AddButton(hwnd, L".", 10, 310, 50, 50, ID_BUTTON_DOT);
        AddButton(hwnd, L"(", 10, 70, 50, 50, ID_BRAKET_LEFT);
        AddButton(hwnd, L")", 70, 70, 50, 50, ID_BRAKET_RIGHT);
        AddButton(hwnd, L"DEL", 190, 70, 50, 50, ID_BUTTON_DEL);

        CreateWindow(
            L"EDIT",
            NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY,
            10, 10, 230, 50,
            hwnd,
            (HMENU)ID_EDIT_RESULT,
            GetModuleHandle(NULL),
            NULL
        );
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BUTTON_0:
        case ID_BUTTON_1:
        case ID_BUTTON_2:
        case ID_BUTTON_3:
        case ID_BUTTON_4:
        case ID_BUTTON_5:
        case ID_BUTTON_6:
        case ID_BUTTON_7:
        case ID_BUTTON_8:
        case ID_BUTTON_9:
        {
            WCHAR digit[2] = { L'0' + LOWORD(wParam) - ID_BUTTON_0, L'\0' };
            AppendTextToEdit(hwnd, digit);

            if (hasOperator) {
                currentNumber.clear();
                hasOperator = false;
            }

            currentNumber += digit;
            isResultDisplayed = false;
        }
        break;

        case ID_BUTTON_DEL:
        {
            // Ïîëó÷àåì òåêóùèé òåêñò èç òåêñòîâîãî ïîëÿ
            HWND hwndEdit = GetDlgItem(hwnd, ID_EDIT_RESULT);
            int len = GetWindowTextLength(hwndEdit);
            std::wstring text(len, L'\0');
            GetWindowText(hwndEdit, &text[0], len + 1);

            // Ïðîâåðÿåì, ÷òî ñòðîêà íå ïóñòàÿ
            if (!text.empty()) {
                // Óäàëÿåì ïîñëåäíèé ñèìâîë èç ñòðîêè
                text.pop_back();

                // Óñòàíàâëèâàåì îáíîâëåííûé òåêñò â òåêñòîâîå ïîëå
                SetWindowText(hwndEdit, text.c_str());
            }
        }
        break;

        case ID_BUTTON_ADD:
            AppendTextToEdit(hwnd, L"+");
            break;

        case ID_BUTTON_DOT:
            AppendTextToEdit(hwnd, L".");
            break;

        case ID_BRAKET_LEFT:
            AppendTextToEdit(hwnd, L"(");
            break;

        case ID_BRAKET_RIGHT:
            AppendTextToEdit(hwnd, L")");
            break;

        case ID_BUTTON_SUBTRACT:
            AppendTextToEdit(hwnd, L"-");
            break;

        case ID_BUTTON_MULTIPLY:
            AppendTextToEdit(hwnd, L"*");
            break;

        case ID_BUTTON_DIVIDE:
            AppendTextToEdit(hwnd, L"/");
            break;

        case ID_BUTTON_CLEAR:
            currentNumber.clear();
            firstNumber.clear();
            lastOperation = L' ';
            hasOperator = false;
            isResultDisplayed = false;
            SetWindowText(GetDlgItem(hwnd, ID_EDIT_RESULT), L"");
            break;

        case ID_BUTTON_EQUALS:
            PerformOperation(hwnd, L'=');
            break;
        case IDM_INFO:
            MessageBox(hwnd, L"Пример простого калькулятора с использованием Win32 API", L"Информация", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_ABOUT:
            MessageBox(hwnd, L"Разработчик: Мамедов Тимур ИС 2-2", L"Информация", MB_OK | MB_ICONINFORMATION);
            break;
        case IDM_EXIT:
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_HOTKEY:
        switch (wParam)
        {
        case 1:
            PerformOperation(hwnd, L'=');
            break;

        case 2:
        {
            // Ïîëó÷àåì òåêóùèé òåêñò èç òåêñòîâîãî ïîëÿ
            HWND hwndEdit = GetDlgItem(hwnd, ID_EDIT_RESULT);
            int len = GetWindowTextLength(hwndEdit);
            std::wstring text(len, L'\0');
            GetWindowText(hwndEdit, &text[0], len + 1);

            // Ïðîâåðÿåì, ÷òî ñòðîêà íå ïóñòàÿ
            if (!text.empty()) {
                // Óäàëÿåì ïîñëåäíèé ñèìâîë èç ñòðîêè
                text.pop_back();

                // Óñòàíàâëèâàåì îáíîâëåííûé òåêñò â òåêñòîâîå ïîëå
                SetWindowText(hwndEdit, text.c_str());
            }
        }
        }
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void AddButton(HWND hwnd, LPCWSTR text, int x, int y, int width, int height, int id) {
    CreateWindow(
        L"BUTTON",
        text,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y, width, height,
        hwnd,
        (HMENU)id,
        GetModuleHandle(NULL),
        NULL
    );
}

void AppendTextToEdit(HWND hwnd, LPCWSTR text) {
    HWND hwndEdit = GetDlgItem(hwnd, ID_EDIT_RESULT);
    int len = GetWindowTextLength(hwndEdit);
    SendMessage(hwndEdit, EM_SETSEL, len, len);
    SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)text);
}

void PerformOperation(HWND hwnd, WCHAR operation) {
    HWND hwndEdit = GetDlgItem(hwnd, ID_EDIT_RESULT);
    int len = GetWindowTextLength(hwndEdit);
    std::wstring expression(len + 1, L'\0');
    GetWindowText(hwndEdit, &expression[0], len + 1);

    try {
        double result = calculate_expression(expression);

        std::wstring result_str = (std::floor(result) == result) ? std::to_wstring(static_cast<int>(result)) : std::to_wstring(result);

        SetWindowText(hwndEdit, result_str.c_str());
    }
    catch (const std::exception& e) {
        MessageBox(hwnd, L"Ошибка при вычислении результата", L"Ошибка", MB_OK | MB_ICONERROR);
        return;
    }
}
