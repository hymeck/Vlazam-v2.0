#include <Windows.h>
#include <windowsx.h>

#include "Vlazam.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR szCmdLine, int nCmdShow);
LRESULT CALLBACK BtnStartRecordingClick();
LRESULT CALLBACK BtnStopRecordingClick();
LRESULT CALLBACK BtnReplayClick();
LRESULT CALLBACK BtnRecognizeClick();

static HWND hBtnStartRecording, hBtnStopRecording, hBtnReplay, hBtnRecognize;

LRESULT CALLBACK BtnStartRecordingClick() {
    if (startRecording() == -1) {
        return EXIT_FAILURE;
    }

    Button_Enable(hBtnStartRecording, FALSE);
    Button_Enable(hBtnStopRecording, TRUE);

    return EXIT_SUCCESS;
}

LRESULT CALLBACK BtnStopRecordingClick() {
    if (stopRecording() == -1) {
        return EXIT_FAILURE;
    }
    if (saveRecording(RECORDED_BUF_FILENAME) == -1) {
        return EXIT_FAILURE;
    }

    Button_Enable(hBtnStartRecording, TRUE);
    Button_Enable(hBtnStopRecording, FALSE);
    Button_Enable(hBtnRecognize, TRUE);
    Button_Enable(hBtnReplay, TRUE);

    return EXIT_SUCCESS;
}

LRESULT CALLBACK BtnReplayClick() {
    int err = 0;
    if ((err = playFileWAV(RECORDED_BUF_FILENAME)) != 0) {
        return err;
    }

    return EXIT_SUCCESS;
}

LRESULT CALLBACK BtnRecognizeClick() {
    int num, res;
    char** results = nullptr;

    if (recognizeSample(results, num) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

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
                WS_OVERLAPPEDWINDOW, 0, 0, 600, 400, nullptr, nullptr,
                wc.hInstance, nullptr)) == INVALID_HANDLE_VALUE)
        return EXIT_FAILURE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int err = 0;
    HINSTANCE hInst;

    switch (uMsg) {
    case WM_CREATE:
        hInst = ((LPCREATESTRUCT)lParam)->hInstance; 
        
        hBtnStartRecording = CreateWindow("button", "Start recording",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            190, 315, 120, 30, hWnd, 0, hInst, NULL);
        ShowWindow(hBtnStartRecording, SW_SHOWNORMAL);
        UpdateWindow(hBtnStartRecording);

        hBtnStopRecording = CreateWindow("button", "Stop recording",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            310, 315, 120, 30, hWnd, 0, hInst, NULL);
        ShowWindow(hBtnStopRecording, SW_SHOWNORMAL);
        UpdateWindow(hBtnStopRecording);
        Button_Enable(hBtnStopRecording, FALSE);

        hBtnReplay = CreateWindow("button", "Replay",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            430, 315, 120, 30, hWnd, 0, hInst, NULL);
        ShowWindow(hBtnReplay, SW_SHOWNORMAL);
        UpdateWindow(hBtnReplay);
        Button_Enable(hBtnReplay, FALSE);

        err = BassDllInit();
        if (err != 0) {
            MessageBox(hWnd, TEXT("Something goes wrong while init"), TEXT("Oops!"), MB_OK);
        }

        break;
        
    case WM_COMMAND: 
        if (lParam == (LPARAM)hBtnStartRecording) {
            if (BtnStartRecordingClick() == EXIT_FAILURE) {
                MessageBox(hWnd, TEXT("Something goes wrong while recording"), TEXT("Oops!"), MB_OK);
            }
        }
        else if (lParam == (LPARAM)hBtnStopRecording) {
            if (BtnStopRecordingClick() == EXIT_FAILURE) {
                MessageBox(hWnd, TEXT("Something goes wrong while stopping."), TEXT("Oops!"), MB_OK);
            }
        }
        else if (lParam == (LPARAM)hBtnReplay) {
            int err = 0;
            if ((err = BtnReplayClick()) != EXIT_SUCCESS) {
                MessageBox(hWnd, TEXT("Something goes wrong while replaying."), TEXT("Oops!"), MB_OK);
            }
        }
        else if (lParam == (LPARAM)hBtnRecognize) {
            if (BtnRecognizeClick() == EXIT_FAILURE) {
            MessageBox(hWnd, TEXT("Something goes wrong while recognizing."), TEXT("Oops!"), MB_OK);
            }
        }
        break;

    case WM_DESTROY:
        err = BassDllCleanup();
        if (err != 0) {
            MessageBox(hWnd, TEXT("Something goes wrong while cleanup"), TEXT("Oops!"), MB_OK);
        }
        PostQuitMessage(EXIT_SUCCESS);
        break;

    default:
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }
    return 0;
};