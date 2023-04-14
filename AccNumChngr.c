#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <ctype.h>

#define LENGTH_OF_ACC_NUMBER 27

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int isAccNumber(char* str, int str_len) {
    for (int i = 0; i < str_len-1; i++) {
        char test = str[i];
        if (str[i] < '0' || str[i] > '9') {
            return 0;
        }
    }
    return 1;
}

char* ACCOUNT_NUMBER = "00111122223333444455556666";
char* TEST = "01010101010101010101010101";

void handle_clipboard_change(HWND hwnd) {
    if (!OpenClipboard(hwnd)) {
        printf("Cannot open clipboard\n");
        return;
    }

    HANDLE hData;
    char* pData = NULL;
    int data_len = 0;
    BOOL isUnicode = FALSE;

    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
        hData = GetClipboardData(CF_UNICODETEXT);
        isUnicode = TRUE;
    }
    else if (IsClipboardFormatAvailable(CF_TEXT)) {
        hData = GetClipboardData(CF_TEXT);
    }
    else {
        printf("Cannot retrieve clipboard data\n");
        CloseClipboard();
        return;
    }

    if (hData == NULL) {
        printf("Cannot retrieve clipboard data\n");
        CloseClipboard();
        return;
    }

    if (isUnicode) {
        wchar_t* pWideData = (wchar_t*)GlobalLock(hData);
        if (pWideData == NULL) {
            printf("Cannot lock clipboard data\n");
            CloseClipboard();
            return;
        }
        data_len = WideCharToMultiByte(CP_UTF8, 0, pWideData, -1, NULL, 0, NULL, NULL);
        pData = (char*)malloc(data_len);
        if (pData == NULL) {
            printf("Cannot allocate memory for clipboard data\n");
            GlobalUnlock(hData);
            CloseClipboard();
            return;
        }
        WideCharToMultiByte(CP_UTF8, 0, pWideData, -1, pData, data_len, NULL, NULL);
        GlobalUnlock(hData);
    }
    else {
        pData = (char*)GlobalLock(hData);
        if (pData == NULL) {
            printf("Cannot lock clipboard data\n");
            CloseClipboard();
            return;
        }
        data_len = strlen(pData);
    }

    if (data_len == LENGTH_OF_ACC_NUMBER && isAccNumber(pData, data_len)) {
        if (!EmptyClipboard()) {
            printf("Cannot empty clipboard\n");
            CloseClipboard();
            return;
        }
        HGLOBAL hNewData = GlobalAlloc(GMEM_FIXED, strlen(ACCOUNT_NUMBER) + 1);
        if (hNewData == NULL) {
            printf("Cannot allocate memory for new data\n");
            CloseClipboard();
            return;
        }
        char* pNewData = (char*)GlobalLock(hNewData);
        strcpy_s(pNewData, strlen(ACCOUNT_NUMBER) + 1, ACCOUNT_NUMBER);
        GlobalUnlock(hNewData);
        SetClipboardData(CF_TEXT, hNewData);
    }

    if (pData != NULL) {
        free(pData);
    }
    CloseClipboard();
}


int main() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wndClass = { 0 };
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = hInstance;
    wndClass.lpszClassName = TEXT("MyWindowClass");
    if (!RegisterClass(&wndClass)) {
        printf("Cannot register window class\n");
        return 1;
    }

    HWND hwnd = CreateWindow(
        TEXT("MyWindowClass"),
        TEXT("MyWindow"),
        0,
        0,
        0,
        0,
        0,
        HWND_MESSAGE,
        NULL,
        hInstance,
        NULL);

    if (!hwnd) {
        printf("Cannot create window\n");
        return 1;
    }

    if (!AddClipboardFormatListener(hwnd)) {
        printf("Cannot add clipboard format listener\n");
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (!RemoveClipboardFormatListener(hwnd)) {
        printf("Cannot remove clipboard format listener\n");
        return 1;
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CLIPBOARDUPDATE:
        handle_clipboard_change(hwnd);
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}