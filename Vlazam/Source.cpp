#include <Windows.h>

#include "Vlazam.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR szCmdLine, int nCmdShow);


int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR szCmdLine, int nCmdShow) {
    MSG msg{};
    HWND hWnd{};
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = TEXT("MyAppClass");
    wc.lpszMenuName = nullptr;
    wc.style = CS_VREDRAW | CS_HREDRAW; //!!!

    if (!RegisterClassEx(&wc))
        return EXIT_FAILURE;

    if ((hWnd = CreateWindow(wc.lpszClassName, TEXT("Vlazam"),
                WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, nullptr, nullptr,
                wc.hInstance, nullptr)) == INVALID_HANDLE_VALUE)
        return EXIT_FAILURE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    if (!initDevice()) {
        return EXIT_FAILURE;
    }

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    SongHash *buf = nullptr;
    char* str = (char*)malloc(50);
    if (!str) {
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }
    strcpy_s(str, 50, DB_DIRECTORY_PATH);
    strcat_s(str, 50, "*");
    int num, res;

    switch (uMsg) {

    case WM_DESTROY:
        PostQuitMessage(EXIT_SUCCESS);//!!!
        break;

    case WM_LBUTTONDOWN:	 
        if (initDevice()) {
            if (startRecording() == -1) {
                return DefWindowProcA(hWnd, uMsg, wParam, lParam);
            }
        }
        break;
    case WM_RBUTTONDOWN:
        if (stopRecording() == -1) {
            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
        }
        if (saveRecording(RECORDED_BUF_FILENAME) == -1) {
            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
        }
        if (playFileWAV(RECORDED_BUF_FILENAME) == -1) {
            return DefWindowProcA(hWnd, uMsg, wParam, lParam);
        }
        break;
        

    default:
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }
    return 0;
};