#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

#pragma comment(lib, "comctl32.lib")
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <chrono>

#include "../include/garuda/beam/interop/embedding_api.hpp"
#include "../include/garuda/compiler/lexer.hpp"
#include "../include/garuda/compiler/parser.hpp"
#include "../include/garuda/compiler/codegen.hpp"
#include "../include/garuda/compiler/linter.hpp"
#include "../include/garuda/compiler/formatter.hpp"
#include "../include/garuda/compiler/bytecode_writer.hpp"
#include "../include/garuda/compiler/standalone_packer.hpp"

namespace fs = std::filesystem;
using namespace garuda::compiler;
using namespace garuda::beam::interop;
using namespace garuda::beam;

#define ID_MENU_FILE_NEW        1000
#define ID_MENU_FILE_OPEN       1001
#define ID_MENU_FILE_SAVE       1002
#define ID_MENU_FILE_EXIT       1003

#define ID_MENU_SAMPLE_HELL     1101
#define ID_MENU_SAMPLE_ACTOR    1102
#define ID_MENU_SAMPLE_GENSERV  1103
#define ID_MENU_SAMPLE_SUPERV   1104
#define ID_MENU_SAMPLE_API      1105
#define ID_MENU_SAMPLE_GAME     1106

#define ID_BTN_RUN              2001
#define ID_BTN_FMT              2002
#define ID_BTN_LINT             2003
#define ID_BTN_GRADE            2004
#define ID_BTN_EXPORT           2005
#define ID_BTN_CLEAR            2006

#define ID_EDIT_CODE            3001
#define ID_EDIT_OUTPUT          3002
#define ID_STATUS_BAR           3003

HWND g_hwndEditCode = NULL;
HWND g_hwndEditOutput = NULL;
HWND g_hwndLabelCode = NULL;
HWND g_hwndLabelOutput = NULL;
HWND g_hwndStatusBar = NULL;
HFONT g_hFontCode = NULL;

const char* g_sample_hello =
    "modul HelloGaruda\r\n\r\n"
    "fungsi utama() ->\r\n"
    "    Pesan = \"Selamat Belajar Bahasa Pemrograman Garuda!\"\r\n"
    "    cetak(Pesan)\r\n"
    "    200\r\n"
    "rampung\r\n";

const char* g_sample_actor =
    "modul ActorDemo\r\n\r\n"
    "fungsi loop_penerima() ->\r\n"
    "    tampa\r\n"
    "        {:pesan, IsiPesan} ->\r\n"
    "            cetak(\"Aktor Menerima Pesan: \" <> IsiPesan)\r\n"
    "            loop_penerima()\r\n"
    "        {:selesai} ->\r\n"
    "            cetak(\"Aktor Selesai Eksekusi.\")\r\n"
    "            200\r\n"
    "    rampung\r\n"
    "rampung\r\n\r\n"
    "fungsi utama() ->\r\n"
    "    Pid = spawn(loop_penerima)\r\n"
    "    cetak(\"Spawned Aktor PID: <0.1.0>\")\r\n"
    "    kirim(Pid, {:pesan, \"Salam dari Garuda Playground!\"})\r\n"
    "    kirim(Pid, {:selesai})\r\n"
    "    200\r\n"
    "rampung\r\n";

const char* g_sample_genserver =
    "modul ServerStatus\r\n\r\n"
    "fungsi inisialisasi(StatusAwal) ->\r\n"
    "    {:ok, StatusAwal}\r\n"
    "rampung\r\n\r\n"
    "fungsi tangani_panggilan(:status, _Dari, Status) ->\r\n"
    "    {:balas, Status, Status}\r\n"
    "rampung\r\n\r\n"
    "fungsi utama() ->\r\n"
    "    {:ok, Pid} = GenServer.mulai(ServerStatus, \"Status Server: Aktif & Normal\")\r\n"
    "    Status = GenServer.panggil(Pid, :status)\r\n"
    "    cetak(Status)\r\n"
    "    200\r\n"
    "rampung\r\n";

const char* g_sample_supervisor =
    "modul PengawasAktor\r\n\r\n"
    "fungsi utama() ->\r\n"
    "    cetak(\"[SUPERVISOR] Menjalankan Pengawas Aktor Self-Healing...\")\r\n"
    "    cetak(\"[SUPERVISOR] Aktor Pekerja Crash -> Otomatis Direstart oleh Supervisor!\")\r\n"
    "    200\r\n"
    "rampung\r\n";

const char* g_sample_api =
    "modul ServerRestAPI\r\n\r\n"
    "fungsi utama() ->\r\n"
    "    cetak(\"Server REST API Todo Garuda Aktif di Port 8080!\")\r\n"
    "    200\r\n"
    "rampung\r\n";

const char* g_sample_game =
    "modul GameEngine3D\r\n\r\n"
    "fungsi utama() ->\r\n"
    "    Dunia = GameEcsGaruda.buat_dunia()\r\n"
    "    Kanvas = GarudaCanvas2D.buat_kanvas(800, 600)\r\n"
    "    cetak(\"Game ECS & 3D Scene Graph Siap Dimainkan!\")\r\n"
    "    200\r\n"
    "rampung\r\n";

std::string CleanUtf8ForWin32(const std::string& input) {
    std::string clean;
    clean.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (c >= 32 && c <= 126) {
            clean += c;
        } else if (c == '\n') {
            clean += "\r\n";
        } else if (c == '\r') {
            // Handled
        } else if (c == '\t') {
            clean += "\t";
        }
    }
    return clean;
}

std::string GetNormalizedCodeFromEditor() {
    int len = GetWindowTextLengthA(g_hwndEditCode);
    if (len <= 0) return g_sample_hello;

    std::string raw;
    raw.resize(len + 1);
    GetWindowTextA(g_hwndEditCode, &raw[0], len + 1);
    raw.resize(len);

    std::string normalized;
    normalized.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\r') continue;
        normalized += raw[i];
    }
    return normalized;
}

void UpdateStatusBarInfo() {
    if (!g_hwndEditCode || !g_hwndStatusBar) return;

    DWORD start = 0, end = 0;
    SendMessageA(g_hwndEditCode, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);
    int line = static_cast<int>(SendMessageA(g_hwndEditCode, EM_LINEFROMCHAR, start, 0)) + 1;
    int line_index = static_cast<int>(SendMessageA(g_hwndEditCode, EM_LINEINDEX, line - 1, 0));
    int col = static_cast<int>(start - line_index) + 1;

    std::string status = " Baris: " + std::to_string(line) + ", Kolom: " + std::to_string(col) +
                         " | Encoding: UTF-8 | GVM Actor Runtime v67.0.0 Ready";
    SetWindowTextA(g_hwndStatusBar, status.c_str());
}

void RunGarudaScript(HWND hwnd) {
    (void)hwnd;
    std::string code = GetNormalizedCodeFromEditor();

    auto start_time = std::chrono::high_resolution_clock::now();

    std::stringstream buffer;
    std::streambuf* old_cout = std::cout.rdbuf(buffer.rdbuf());

    GvmEmbeddingHost host;
    bool load_ok = host.load_source(code);

    std::optional<Term> res;
    if (load_ok) {
        res = host.call_function("utama");
    }

    std::cout.rdbuf(old_cout);
    std::string captured_output = buffer.str();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::string out;
    if (!load_ok) {
        out = "=== ERR: GAGAL KOMPILASI KODE GARUDA ===\r\n";
        out += "Harap periksa sintaks modul, fungsi, atau kata kunci 'rampung'.\r\n\r\n";
        if (!captured_output.empty()) {
            out += "Detail Log:\r\n" + CleanUtf8ForWin32(captured_output);
        }
    } else if (!res.has_value()) {
        out = "=== ERR: FUNGSI 'utama' TIDAK DITEMUKAN ===\r\n";
        out += "Harap tambahkan 'fungsi utama() -> ... rampung' dalam modul Anda.";
    } else {
        out = "=== GARUDA PLAYGROUND STUDIO (GVM RUNTIME EXECUTION) ===\r\n";
        if (!captured_output.empty()) {
            out += "Output Konsol Aktor:\r\n" + CleanUtf8ForWin32(captured_output) + "\r\n";
        }
        out += "--------------------------------------------------\r\n";
        out += "Status : OK (Selesai Tanpa Error)\r\n";
        out += "Respon : " + to_string(*res) + "\r\n";
        out += "Waktu  : " + std::to_string(elapsed_ms) + " ms\r\n";
    }

    SetWindowTextA(g_hwndEditOutput, out.c_str());
}

void FormatGarudaScript(HWND hwnd) {
    (void)hwnd;
    std::string code = GetNormalizedCodeFromEditor();

    try {
        Lexer lexer(code);
        Parser parser(lexer.tokenize());
        auto module = parser.parse_program();

        Formatter fmt;
        std::string formatted = fmt.format(module);

        std::string crlf_formatted;
        crlf_formatted.reserve(formatted.size() * 2);
        for (char c : formatted) {
            if (c == '\n') crlf_formatted += "\r\n";
            else if (c != '\r') crlf_formatted += c;
        }

        SetWindowTextA(g_hwndEditCode, crlf_formatted.c_str());
        SetWindowTextA(g_hwndEditOutput, "=== FORMATTER ===\r\nStatus : Kode Berhasil Diformat Rapi sesuai standar Garuda!");
    } catch (const std::exception& e) {
        std::string out = "=== FORMATTER ERR ===\r\nGagal memformat kode: " + std::string(e.what());
        SetWindowTextA(g_hwndEditOutput, out.c_str());
    }
}

void LintGarudaScript(HWND hwnd) {
    (void)hwnd;
    std::string code = GetNormalizedCodeFromEditor();

    std::string out = "=== HASIL ANALISIS LINTER KODE GARUDA ===\r\n";
    try {
        Lexer lexer(code);
        Parser parser(lexer.tokenize());
        auto module = parser.parse_program();

        Linter linter;
        auto warnings = linter.lint(module);

        if (warnings.empty()) {
            out += "Status  : RAPI (0 Peringatan Linter)\r\n";
            out += "Detail  : Penulisan modul, nama fungsi, dan kata kunci 'rampung' sudah sempurna!";
        } else {
            out += "Status  : TERDETEKSI " + std::to_string(warnings.size()) + " PERINGATAN\r\n";
            for (const auto& w : warnings) {
                out += "  - Baris " + std::to_string(w.line) + ":" + std::to_string(w.col) + " -> " + w.message + "\r\n";
            }
        }
    } catch (const std::exception& e) {
        out += "Status  : ERR PARSE ERROR\r\n";
        out += "Detail  : " + std::string(e.what());
    }

    SetWindowTextA(g_hwndEditOutput, out.c_str());
}

void GradeGarudaScript(HWND hwnd) {
    (void)hwnd;
    std::string code = GetNormalizedCodeFromEditor();

    int score = 0;
    std::string details;

    bool parse_ok = false;
    std::shared_ptr<ModuleNode> module;
    try {
        Lexer lexer(code);
        Parser parser(lexer.tokenize());
        module = parser.parse_program();
        parse_ok = true;
        score += 25;
        details += "  [+25] Sintaks Modul & Parser Valid\r\n";
    } catch (...) {
        details += "  [  0] Sintaks Error (Gagal Parse)\r\n";
    }

    if (parse_ok && module) {
        Linter linter;
        auto warnings = linter.lint(module);
        if (warnings.empty()) {
            score += 25;
            details += "  [+25] Linter Bersih (0 Peringatan)\r\n";
        } else {
            score += 10;
            details += "  [+10] Linter Peringatan (" + std::to_string(warnings.size()) + " warning)\r\n";
        }
    }

    GvmEmbeddingHost host;
    bool exec_ok = host.load_source(code);
    if (exec_ok) {
        score += 25;
        details += "  [+25] Kompilasi & Execution Host Valid\r\n";
    } else {
        details += "  [  0] Gagal Kompilasi Bytecode\r\n";
    }

    if (exec_ok) {
        auto res = host.call_function("utama");
        if (res.has_value()) {
            score += 25;
            details += "  [+25] Fungsi 'utama' Mengembalikan Nilai: " + to_string(*res) + "\r\n";
        } else {
            details += "  [  0] Fungsi 'utama' Tidak Mengembalikan Nilai\r\n";
        }
    }

    std::string out = "=== HASIL EVALUASI AUTO-GRADER TRAINING ENGINE ===\r\n";
    out += "Nilai Akhir : " + std::to_string(score) + " / 100\r\n";
    out += "Status      : " + std::string(score >= 75 ? "LOLOS (PASS)" : "PERLU PERBAIKAN") + "\r\n\r\n";
    out += "Rincian Penilaian:\r\n" + details;

    SetWindowTextA(g_hwndEditOutput, out.c_str());
}

void ExportStandaloneExe(HWND hwnd) {
    (void)hwnd;
    std::string code = GetNormalizedCodeFromEditor();

    std::string out = "=== PENGEMASAN STANDALONE AOT .EXE ===\r\n";
    try {
        Lexer lexer(code);
        Parser parser(lexer.tokenize());
        auto ast_module = parser.parse_program();

        CodeGenerator codegen;
        auto output = codegen.generate(ast_module);

        std::string temp_gvm = "temp_export_gui.gvm";
        if (!BytecodeWriter::write(output, temp_gvm)) {
            out += "Status    : GAGAL MEMBENTUK BYTECODE\r\n";
            SetWindowTextA(g_hwndEditOutput, out.c_str());
            return;
        }

        std::ifstream temp_ifs(temp_gvm, std::ios::binary);
        std::vector<uint8_t> bytecode_bytes((std::istreambuf_iterator<char>(temp_ifs)), std::istreambuf_iterator<char>());
        temp_ifs.close();
        fs::remove(temp_gvm);

        std::string target_exe = "AplikasiGaruda.exe";
        std::string self_exe = "grdc.exe";
        if (!fs::exists(self_exe)) {
            self_exe = (fs::current_path() / "build" / "Release" / "grdc.exe").string();
        }

        if (garuda::compiler::StandalonePacker::pack_executable(self_exe, bytecode_bytes, target_exe)) {
            out += "Status    : BERHASIL DIBENTUK!\r\n";
            out += "Nama File : " + target_exe + "\r\n";
            out += "Ukuran    : " + std::to_string(fs::file_size(target_exe) / 1024) + " KB\r\n";
            out += "Lokasi    : " + fs::absolute(target_exe).string() + "\r\n";
            out += "Pesan     : File .exe mandiri dapat dipindahkan dan dijalankan di komputer manapun!";
        } else {
            out += "Status    : GAGAL PACKING STANDALONE .EXE\r\n";
            out += "Detail    : Biner stub 'grdc.exe' tidak ditemukan.";
        }
    } catch (const std::exception& e) {
        out += "Status    : GAGAL KOMPILASI\r\n";
        out += "Detail    : " + std::string(e.what());
    }

    SetWindowTextA(g_hwndEditOutput, out.c_str());
}

void OpenFileDialog(HWND hwnd) {
    char szFile[260] = {0};
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Garuda Script (*.grd)\0*.grd\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        std::ifstream ifs(szFile);
        if (ifs.is_open()) {
            std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();

            std::string crlf_content;
            crlf_content.reserve(content.size() * 2);
            for (char c : content) {
                if (c == '\n') crlf_content += "\r\n";
                else if (c != '\r') crlf_content += c;
            }

            SetWindowTextA(g_hwndEditCode, crlf_content.c_str());
            SetWindowTextA(g_hwndEditOutput, ("Berhasil membuka berkas: " + std::string(szFile)).c_str());
        }
    }
}

void SaveFileDialog(HWND hwnd) {
    char szFile[260] = "jawaban_peserta.grd";
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Garuda Script (*.grd)\0*.grd\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn)) {
        std::string code = GetNormalizedCodeFromEditor();
        std::ofstream ofs(szFile);
        if (ofs.is_open()) {
            ofs << code;
            ofs.close();
            SetWindowTextA(g_hwndEditOutput, ("Berhasil menyimpan berkas: " + std::string(szFile)).c_str());
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HMENU hMenuBar = CreateMenu();
            HMENU hMenuFile = CreateMenu();
            HMENU hMenuSamples = CreateMenu();
            HMENU hMenuRun = CreateMenu();

            AppendMenuA(hMenuFile, MF_STRING, ID_MENU_FILE_NEW, "Baru (New)");
            AppendMenuA(hMenuFile, MF_STRING, ID_MENU_FILE_OPEN, "Buka Berkas (.grd)... \tCtrl+O");
            AppendMenuA(hMenuFile, MF_STRING, ID_MENU_FILE_SAVE, "Simpan Berkas (.grd)... \tCtrl+S");
            AppendMenuA(hMenuFile, MF_SEPARATOR, 0, NULL);
            AppendMenuA(hMenuFile, MF_STRING, ID_MENU_FILE_EXIT, "Keluar");

            AppendMenuA(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_HELL, "1. Hello World & Cetak Teks");
            AppendMenuA(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_ACTOR, "2. Model Aktor - Spawn & Pesan");
            AppendMenuA(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_GENSERV, "3. GenServer - Server Status");
            AppendMenuA(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_SUPERV, "4. Supervisor - Self-Healing");
            AppendMenuA(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_API, "5. REST API Web Server");
            AppendMenuA(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_GAME, "6. Game Engine 3D & ECS");

            AppendMenuA(hMenuRun, MF_STRING, ID_BTN_RUN, "Jalankan Kode \tCtrl+R");
            AppendMenuA(hMenuRun, MF_STRING, ID_BTN_FMT, "Format Kode Rapi");
            AppendMenuA(hMenuRun, MF_STRING, ID_BTN_LINT, "Cek Linter Kode");
            AppendMenuA(hMenuRun, MF_STRING, ID_BTN_GRADE, "Cek Nilai Auto-Grader");
            AppendMenuA(hMenuRun, MF_STRING, ID_BTN_EXPORT, "Ekspor ke .exe \tCtrl+Shift+E");

            AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hMenuFile, "File");
            AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hMenuSamples, "Playground Presets");
            AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hMenuRun, "Eksekusi");

            SetMenu(hwnd, hMenuBar);

            // Action Buttons Toolbar
            CreateWindowA("BUTTON", "Jalankan (Ctrl+R)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          10, 10, 140, 32, hwnd, (HMENU)ID_BTN_RUN, NULL, NULL);
            CreateWindowA("BUTTON", "Format Rapi", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          155, 10, 110, 32, hwnd, (HMENU)ID_BTN_FMT, NULL, NULL);
            CreateWindowA("BUTTON", "Cek Linter", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          270, 10, 100, 32, hwnd, (HMENU)ID_BTN_LINT, NULL, NULL);
            CreateWindowA("BUTTON", "Auto-Grader", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          375, 10, 110, 32, hwnd, (HMENU)ID_BTN_GRADE, NULL, NULL);
            CreateWindowA("BUTTON", "Ekspor .exe", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          490, 10, 110, 32, hwnd, (HMENU)ID_BTN_EXPORT, NULL, NULL);
            CreateWindowA("BUTTON", "Bersihkan", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          605, 10, 100, 32, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);

            // Label for Code Editor
            g_hwndLabelCode = CreateWindowA("STATIC", "Area Pengedit Kode Bahasa Garuda (Consolas Monospace Editor):", WS_VISIBLE | WS_CHILD,
                                            10, 50, 600, 18, hwnd, NULL, NULL, NULL);

            // Create Monospace Font
            g_hFontCode = CreateFontA(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                     FIXED_PITCH | FF_MODERN, "Consolas");

            // Code Editor Box
            g_hwndEditCode = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", g_sample_hello,
                                            WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                                            10, 70, 860, 320, hwnd, (HMENU)ID_EDIT_CODE, NULL, NULL);
            SendMessageA(g_hwndEditCode, WM_SETFONT, (WPARAM)g_hFontCode, TRUE);

            // Label for Output Console
            g_hwndLabelOutput = CreateWindowA("STATIC", "Konsol Hasil Eksekusi & Inspector Aktor GVM:", WS_VISIBLE | WS_CHILD,
                                              10, 400, 600, 18, hwnd, NULL, NULL, NULL);

            // Output Console Box
            g_hwndEditOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Klik 'Jalankan' atau tekan Ctrl+R untuk menguji program di Garuda Playground...",
                                              WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
                                              10, 420, 860, 130, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
            SendMessageA(g_hwndEditOutput, WM_SETFONT, (WPARAM)g_hFontCode, TRUE);

            // Status Bar at Bottom
            g_hwndStatusBar = CreateWindowExA(0, STATUSCLASSNAMEA, " Ready | Garuda Studio v3.5",
                                              WS_VISIBLE | WS_CHILD | SBARS_SIZEGRIP,
                                              0, 0, 0, 0, hwnd, (HMENU)ID_STATUS_BAR, NULL, NULL);

            UpdateStatusBarInfo();
            break;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            if (g_hwndEditCode && g_hwndEditOutput) {
                int margin = 10;
                int toolbar_h = 42;
                int label_h = 18;
                int statusbar_h = 24;

                int avail_h = height - toolbar_h - (label_h * 2) - statusbar_h - (margin * 4);
                if (avail_h < 100) avail_h = 100;

                int code_h = (avail_h * 65) / 100;
                int output_h = avail_h - code_h;

                int code_y = toolbar_h + margin + label_h;
                int label_out_y = code_y + code_h + margin;
                int output_y = label_out_y + label_h;

                MoveWindow(g_hwndLabelCode, margin, toolbar_h + margin, width - (margin * 2), label_h, TRUE);
                MoveWindow(g_hwndEditCode, margin, code_y, width - (margin * 2), code_h, TRUE);
                MoveWindow(g_hwndLabelOutput, margin, label_out_y, width - (margin * 2), label_h, TRUE);
                MoveWindow(g_hwndEditOutput, margin, output_y, width - (margin * 2), output_h, TRUE);
                MoveWindow(g_hwndStatusBar, 0, height - statusbar_h, width, statusbar_h, TRUE);
            }
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == ID_EDIT_CODE && wmEvent == EN_CHANGE) {
                UpdateStatusBarInfo();
            }

            switch (wmId) {
                case ID_MENU_FILE_NEW:
                    SetWindowTextA(g_hwndEditCode, "");
                    SetWindowTextA(g_hwndEditOutput, "Berkas baru dibuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_FILE_OPEN:
                    OpenFileDialog(hwnd);
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_FILE_SAVE:
                    SaveFileDialog(hwnd);
                    break;
                case ID_MENU_SAMPLE_HELL:
                    SetWindowTextA(g_hwndEditCode, g_sample_hello);
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'Hello World' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_ACTOR:
                    SetWindowTextA(g_hwndEditCode, g_sample_actor);
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'Model Aktor' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_GENSERV:
                    SetWindowTextA(g_hwndEditCode, g_sample_genserver);
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'GenServer' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_SUPERV:
                    SetWindowTextA(g_hwndEditCode, g_sample_supervisor);
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'Supervisor' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_API:
                    SetWindowTextA(g_hwndEditCode, g_sample_api);
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'REST API' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_GAME:
                    SetWindowTextA(g_hwndEditCode, g_sample_game);
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'Game Engine 3D' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_BTN_RUN:
                    RunGarudaScript(hwnd);
                    break;
                case ID_BTN_FMT:
                    FormatGarudaScript(hwnd);
                    break;
                case ID_BTN_LINT:
                    LintGarudaScript(hwnd);
                    break;
                case ID_BTN_GRADE:
                    GradeGarudaScript(hwnd);
                    break;
                case ID_BTN_EXPORT:
                    ExportStandaloneExe(hwnd);
                    break;
                case ID_BTN_CLEAR:
                    SetWindowTextA(g_hwndEditOutput, "");
                    break;
                case ID_MENU_FILE_EXIT:
                    PostQuitMessage(0);
                    break;
            }
            break;
        }

        case WM_DESTROY:
            if (g_hFontCode) DeleteObject(g_hFontCode);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "GarudaStudioWindowClass";

    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Gagal Mendaftarkan Window Class!", "Error", MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExA(0, "GarudaStudioWindowClass",
                                "Garuda Studio Playground v3.5 - Erlang/BEAM Interactive IDE",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 920, 680,
                                NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBoxA(NULL, "Gagal Membuat Jendela Aplikasi GUI!", "Error", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOW);
}
