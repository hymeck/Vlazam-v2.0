#include <Windows.h>
#include <windowsx.h>
#include <shobjidl.h>
#include "Vlazam.h"
#include <string>

#define STATIC_RESULTS_INTRO "There are songs suitable for your request:"
#define IDM_ADDTODATABASE 0
#define IDM_EXIT 1
#define IDM_ABOUT 2


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR szCmdLine, int nCmdShow);
LRESULT CALLBACK BtnStartRecordingClick();
LRESULT CALLBACK BtnStopRecordingClick();
LRESULT CALLBACK BtnReplayClick();
LRESULT CALLBACK BtnRecognizeClick();
LRESULT CALLBACK MenuAddToDatabaseClick();
HWND CreateButton(HWND hWnd, HINSTANCE hInst, const char* btnCapture, BOOL isEnabled, int x, int y, int width, int height);
HWND CreateStatic(HWND hWnd, HINSTANCE hInst, const char* btnCapture, int x, int y, int width, int height);

static HWND hBtnStartRecording, hBtnStopRecording, hBtnReplay, hBtnRecognize, hStaticStatus, hStaticResults;
HMENU hMenu, hFileMenu, hHelpMenu;

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

    if (!RegisterClassEx(&wc)) {
        return EXIT_FAILURE;
    }

    if ((hWnd = CreateWindow(wc.lpszClassName, TEXT("Vlazam"),
            WS_OVERLAPPEDWINDOW, 0, 0, 600, 210, nullptr, nullptr,
            wc.hInstance, nullptr)) == INVALID_HANDLE_VALUE) {
        return EXIT_FAILURE;
    }

    // Create menubar
    hFileMenu = CreatePopupMenu();
    AppendMenu(hFileMenu, MF_ENABLED | MF_STRING, IDM_ADDTODATABASE, TEXT("Add to database"));
    AppendMenu(hFileMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(hFileMenu, MF_ENABLED | MF_STRING, IDM_EXIT, TEXT("Exit"));
    hHelpMenu = CreatePopupMenu();
    AppendMenu(hHelpMenu, MF_DISABLED | MF_STRING, IDM_ABOUT, TEXT("About"));
    hMenu = CreateMenu();
    AppendMenu(hMenu, MF_ENABLED | MF_POPUP | MF_STRING, (UINT)hFileMenu, TEXT("File"));
    AppendMenu(hMenu, MF_ENABLED | MF_POPUP | MF_STRING, (UINT)hHelpMenu, TEXT("Help"));
    SetMenu(hWnd, hMenu);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    DrawMenuBar(hWnd);

    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int err = 0;
    UINT_PTR intPtr = 0;
    HINSTANCE hInst;

    switch (uMsg) {
    case WM_CREATE:
        hInst = ((LPCREATESTRUCT)lParam)->hInstance;

        hBtnStartRecording = CreateButton(hWnd, hInst, "Start recording", TRUE, 215, 115, 120, 30);

        hBtnStopRecording = CreateButton(hWnd, hInst, "Stop recording", FALSE, 335, 115, 120, 30);

        hBtnReplay = CreateButton(hWnd, hInst, "Replay", FALSE, 455, 115, 120, 30);

        hBtnRecognize = CreateButton(hWnd, hInst, "Recognize", FALSE, 10, 10, 120, 30);

        hStaticStatus = CreateStatic(hWnd, hInst, "", 5, 135, 200, 20);

        hStaticResults = CreateStatic(hWnd, hInst, "", 5, 50, 550, 50);
        
        if ((err = BassDllInit()) != 0) {
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
            if ((err = BtnReplayClick()) != EXIT_SUCCESS) {
                MessageBox(hWnd, TEXT("Something goes wrong while replaying."), TEXT("Oops!"), MB_OK);
            }
        }
        else if (lParam == (LPARAM)hBtnRecognize) {
            if (BtnRecognizeClick() == EXIT_FAILURE) {
                MessageBox(hWnd, TEXT("Something goes wrong while recognizing."), TEXT("Oops!"), MB_OK);
            }
        }
        // otherwise it is message from menubar
        else {
            switch (LOWORD(wParam)) {
            case IDM_ABOUT:
                MessageBox(hWnd, TEXT("Help me"), TEXT("Plz"), MB_OK);
                break;

            case IDM_ADDTODATABASE:
                if (MenuAddToDatabaseClick() == -1) {
                    MessageBox(hWnd, TEXT("Something goes wrong while adding to database."), TEXT("Oops!"), MB_OK);
                }
                break;

            case IDM_EXIT: 
                SendMessage(hWnd, WM_CLOSE, NULL, NULL);
                break;
            }
        }
        break;

    case WM_DESTROY:
        err = BassDllCleanup();
        if (err != 0) {
            MessageBox(hWnd, TEXT("Something goes wrong while cleanup"), TEXT("Oops!"), MB_OK);
        }
        DestroyMenu(hMenu);
        PostQuitMessage(EXIT_SUCCESS);
        break;

    default:
        return DefWindowProcA(hWnd, uMsg, wParam, lParam);
    }
    return 0;
};

HWND CreateStatic(HWND hWnd, HINSTANCE hInst, const char* btnCapture, int x, int y, int width, int height) {
    HWND sttc = CreateWindow("static", "",
        WS_CHILD | SS_LEFTNOWORDWRAP,
        x, y, width, height, hWnd, 0, hInst, NULL);
    ShowWindow(sttc, SW_SHOWNORMAL);
    UpdateWindow(sttc);
    return sttc;
}

HWND CreateButton(HWND hWnd, HINSTANCE hInst, const char* btnCapture, BOOL isEnabled, int x, int y, int width, int height) {
    HWND btn = CreateWindow("button", btnCapture,
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, width, height, hWnd, 0, hInst, NULL);
    ShowWindow(btn, SW_SHOWNORMAL);
    UpdateWindow(btn);
    Button_Enable(btn, isEnabled);
    return btn;
}

LRESULT CALLBACK MenuAddToDatabaseClick() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (!SUCCEEDED(hr)) {
        return EXIT_FAILURE;
    }

    IFileOpenDialog* pFileOpen;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

    if (!SUCCEEDED(hr)) {
        return EXIT_FAILURE;
    }

    COMDLG_FILTERSPEC rgSpec[] =
    {
        { L"", L"*.wav" },
    };
    hr = pFileOpen->SetFileTypes(1, rgSpec);

    if (!SUCCEEDED(hr)) {
        return EXIT_FAILURE;
    }

    // Show the Open dialog box.
    hr = pFileOpen->Show(NULL);

    if (!SUCCEEDED(hr)) {
        return EXIT_FAILURE;
    }

    // Get the file name from the dialog box.
    IShellItem* pItem;
    hr = pFileOpen->GetResult(&pItem);

    if (!SUCCEEDED(hr)) {
        return EXIT_FAILURE;
    }

    PWSTR pszFilePath;
    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

    if (!SUCCEEDED(hr)) {
        return EXIT_FAILURE;
    }
    
    // now convert WCHAR to CHAR
    size_t wLen = wcslen(pszFilePath);
    char *fileName = (char*)malloc(sizeof(char) * (wLen + 1));
    if (!fileName) {
        return EXIT_FAILURE;
    }
    size_t cLen = 0;
    int err = wcstombs_s(&cLen, fileName, wLen + 1, pszFilePath, wLen + 1);
    if (err) {
        return EXIT_FAILURE;
    }
    fileName[cLen] = '\0';


    // cleanup
    pItem->Release();
    pFileOpen->Release();
    CoUninitialize();
    return addToDatabase(fileName);
}

LRESULT CALLBACK BtnStartRecordingClick() {
    if (startRecording() == -1) {
        return EXIT_FAILURE;
    }

    Static_SetText(hStaticStatus, "Recording...");

    Button_Enable(hBtnStartRecording, FALSE);
    Button_Enable(hBtnStopRecording, TRUE);

    Button_Enable(hBtnRecognize, FALSE);
    Button_Enable(hBtnReplay, FALSE);

    return EXIT_SUCCESS;
}

LRESULT CALLBACK BtnStopRecordingClick() {
    if (stopRecording() == -1) {
        return EXIT_FAILURE;
    }
    if (saveRecording(RECORDED_BUF_FILENAME) == -1) {
        return EXIT_FAILURE;
    }

    Static_SetText(hStaticStatus, "Ready to replay/recognise.");

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
    Static_SetText(hStaticStatus, "Replaying...");
    Button_Enable(hBtnStartRecording, FALSE);
    Button_Enable(hBtnStopRecording, FALSE);
    Button_Enable(hBtnRecognize, FALSE);
    Button_Enable(hBtnReplay, FALSE);
    waitTillPlaying();
    Button_Enable(hBtnStartRecording, TRUE);
    Button_Enable(hBtnStopRecording, FALSE);
    Button_Enable(hBtnRecognize, TRUE);
    Button_Enable(hBtnReplay, TRUE);
    Static_SetText(hStaticStatus, "Ready to replay/recognise.");

    return EXIT_SUCCESS;
}

LRESULT CALLBACK BtnRecognizeClick() {
    int num;;
    char** results = nullptr;

    Static_SetText(hStaticStatus, "Recognizing...");

    if (recognizeSample(results, num) == -1) {
        Static_SetText(hStaticStatus, "Some errors occur.");
        return EXIT_FAILURE;
    }

    int staticSumLen = 0;
    for (int i = 0; i < num; i++) {
        staticSumLen += strlen(results[i]) + 2;
    }
    staticSumLen += strlen(STATIC_RESULTS_INTRO) + 1;
    char* staticResultsText = (char*)malloc(sizeof(char) * staticSumLen);
    if (!staticResultsText) {
        return EXIT_FAILURE;
    }
    memset(staticResultsText, 0, staticSumLen);
    strcpy_s(staticResultsText, staticSumLen, STATIC_RESULTS_INTRO);
    for (int i = 0; i < num; i++) {
        strcat_s(staticResultsText, staticSumLen, "\n\t");
        strcat_s(staticResultsText, staticSumLen, results[i]);
    }

    Static_SetText(hStaticResults, staticResultsText);

    Static_SetText(hStaticStatus, "Ready to replay/recognise.");

    return EXIT_SUCCESS;
}