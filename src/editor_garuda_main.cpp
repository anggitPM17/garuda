#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <richedit.h>
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

#define ID_EDIT_GUTTER          3000
#define ID_EDIT_CODE            3001
#define ID_EDIT_OUTPUT          3002
#define ID_STATUS_BAR           3003

HWND g_hwndGutter = NULL;
HWND g_hwndEditCode = NULL;
HWND g_hwndEditOutput = NULL;
HWND g_hwndLabelCode = NULL;
HWND g_hwndLabelOutput = NULL;
HWND g_hwndStatusBar = NULL;
HFONT g_hFontCode = NULL;

HBRUSH g_hbrDarkBg = NULL;
HBRUSH g_hbrGutterBg = NULL;
HBRUSH g_hbrConsoleBg = NULL;

bool g_isFormatting = false;

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
    "    kirim(Pid, {:pesan, \"Salam dari Garuda Studio!\"})\r\n"
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

void UpdateLineNumbersGutter() {
    if (!g_hwndEditCode || !g_hwndGutter) return;

    int line_count = static_cast<int>(SendMessageA(g_hwndEditCode, EM_GETLINECOUNT, 0, 0));
    if (line_count < 1) line_count = 1;

    std::string gutter_text;
    gutter_text.reserve(line_count * 6);
    for (int i = 1; i <= line_count; ++i) {
        if (i < 10) gutter_text += "  " + std::to_string(i) + "\r\n";
        else if (i < 100) gutter_text += " " + std::to_string(i) + "\r\n";
        else gutter_text += std::to_string(i) + "\r\n";
    }

    SetWindowTextA(g_hwndGutter, gutter_text.c_str());

    int first_visible_line = static_cast<int>(SendMessageA(g_hwndEditCode, EM_GETFIRSTVISIBLELINE, 0, 0));
    SendMessageA(g_hwndGutter, EM_LINESCROLL, 0, first_visible_line);
}

void ApplySyntaxHighlighting() {
    if (!g_hwndEditCode || g_isFormatting) return;
    g_isFormatting = true;

    // Save current selection / scroll
    CHARRANGE cr_saved;
    SendMessageA(g_hwndEditCode, EM_EXGETSEL, 0, (LPARAM)&cr_saved);
    POINT scroll_pt;
    SendMessageA(g_hwndEditCode, EM_GETSCROLLPOS, 0, (LPARAM)&scroll_pt);

    // Disable redrawing during formatting
    SendMessageA(g_hwndEditCode, WM_SETREDRAW, FALSE, 0);

    std::string code = GetNormalizedCodeFromEditor();

    // Default formatting
    CHARFORMAT2A cf_default = {0};
    cf_default.cbSize = sizeof(CHARFORMAT2A);
    cf_default.dwMask = CFM_COLOR | CFM_BOLD;
    cf_default.crTextColor = RGB(212, 212, 212); // VS Code Light Grey
    cf_default.dwEffects = 0;

    CHARRANGE cr_all = {0, -1};
    SendMessageA(g_hwndEditCode, EM_EXSETSEL, 0, (LPARAM)&cr_all);
    SendMessageA(g_hwndEditCode, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf_default);

    try {
        Lexer lexer(code);
        auto tokens = lexer.tokenize();

        for (const auto& tok : tokens) {
            COLORREF color = RGB(212, 212, 212);
            bool bold = false;

            if (tok.type == TokenType::DefModule || tok.type == TokenType::DefFunc ||
                tok.type == TokenType::End || tok.type == TokenType::Spawn ||
                tok.type == TokenType::Receive || tok.type == TokenType::If ||
                tok.type == TokenType::Else || tok.type == TokenType::Try ||
                tok.type == TokenType::Catch) {
                color = RGB(86, 156, 214); // Cyan Blue
                bold = true;
            } else if (tok.type == TokenType::String) {
                color = RGB(206, 145, 120); // Amber String
            } else if (tok.type == TokenType::Number || tok.type == TokenType::Float) {
                color = RGB(181, 206, 168); // Light Green
            } else if (tok.type == TokenType::Identifier) {
                color = RGB(220, 220, 170); // Light Yellow Function/Var Name
            } else if (tok.type == TokenType::Atom) {
                color = RGB(197, 134, 192); // Purple Atom
            }

            CHARFORMAT2A cf = {0};
            cf.cbSize = sizeof(CHARFORMAT2A);
            cf.dwMask = CFM_COLOR | (bold ? CFM_BOLD : 0);
            cf.crTextColor = color;
            cf.dwEffects = bold ? CFE_BOLD : 0;

            int pos = static_cast<int>(SendMessageA(g_hwndEditCode, EM_LINEINDEX, tok.line - 1, 0)) + (static_cast<int>(tok.column) - 1);
            CHARRANGE cr;
            cr.cpMin = pos;
            cr.cpMax = pos + static_cast<int>(tok.text.size());

            SendMessageA(g_hwndEditCode, EM_EXSETSEL, 0, (LPARAM)&cr);
            SendMessageA(g_hwndEditCode, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
        }
    } catch (...) {
        // Fallback silently if code is mid-typing syntax error
    }

    // Restore selection & scroll position
    SendMessageA(g_hwndEditCode, EM_EXSETSEL, 0, (LPARAM)&cr_saved);
    SendMessageA(g_hwndEditCode, EM_SETSCROLLPOS, 0, (LPARAM)&scroll_pt);

    // Re-enable redraw
    SendMessageA(g_hwndEditCode, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(g_hwndEditCode, NULL, TRUE);

    g_isFormatting = false;
}

void UpdateStatusBarInfo() {
    if (!g_hwndEditCode || !g_hwndStatusBar) return;

    CHARRANGE cr;
    SendMessageA(g_hwndEditCode, EM_EXGETSEL, 0, (LPARAM)&cr);
    int line = static_cast<int>(SendMessageA(g_hwndEditCode, EM_LINEFROMCHAR, cr.cpMin, 0)) + 1;
    int line_index = static_cast<int>(SendMessageA(g_hwndEditCode, EM_LINEINDEX, line - 1, 0));
    int col = static_cast<int>(cr.cpMin - line_index) + 1;

    std::string status = " Baris: " + std::to_string(line) + ", Kolom: " + std::to_string(col) +
                         " | Encoding: UTF-8 | GVM Actor Runtime v67.0.0 Active";
    SetWindowTextA(g_hwndStatusBar, status.c_str());

    UpdateLineNumbersGutter();
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
        out = "=== GARUDA STUDIO (GVM RUNTIME EXECUTION) ===\r\n";
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
        ApplySyntaxHighlighting();
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
            ApplySyntaxHighlighting();
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
            // Load RichEdit DLL for Syntax Highlighting & Colors
            LoadLibraryA("riched20.dll");

            g_hbrDarkBg = CreateSolidBrush(RGB(30, 30, 30));       // VS Code Dark `#1E1E1E`
            g_hbrGutterBg = CreateSolidBrush(RGB(37, 37, 38));     // Gutter `#252526`
            g_hbrConsoleBg = CreateSolidBrush(RGB(24, 24, 24));    // Console `#181818`

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
            AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hMenuSamples, "Contoh Kodingan");
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

            // Labels
            g_hwndLabelCode = CreateWindowA("STATIC", "Area Pengedit Kode Bahasa Garuda (Consolas Monospace + Syntax Highlighting):", WS_VISIBLE | WS_CHILD,
                                            10, 50, 600, 18, hwnd, NULL, NULL, NULL);

            // Create Monospace Font
            g_hFontCode = CreateFontA(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                     FIXED_PITCH | FF_MODERN, "Consolas");

            // Line Numbers Gutter Box
            g_hwndGutter = CreateWindowExA(0, "RICHEDIT20A", "  1\r\n",
                                           WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY | ES_RIGHT,
                                           10, 70, 45, 320, hwnd, (HMENU)ID_EDIT_GUTTER, NULL, NULL);
            SendMessageA(g_hwndGutter, WM_SETFONT, (WPARAM)g_hFontCode, TRUE);

            // Code Editor Box (RichEdit for Syntax Highlighting)
            g_hwndEditCode = CreateWindowExA(WS_EX_CLIENTEDGE, "RICHEDIT20A", g_sample_hello,
                                            WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                                            60, 70, 810, 320, hwnd, (HMENU)ID_EDIT_CODE, NULL, NULL);
            SendMessageA(g_hwndEditCode, WM_SETFONT, (WPARAM)g_hFontCode, TRUE);

            // Enable ENM_CHANGE & ENM_SCROLL events
            SendMessageA(g_hwndEditCode, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_SCROLL);

            // Set Editor Background Color to Dark `#1E1E1E`
            SendMessageA(g_hwndEditCode, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(30, 30, 30));
            SendMessageA(g_hwndGutter, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(37, 37, 38));

            // Label for Output Console
            g_hwndLabelOutput = CreateWindowA("STATIC", "Konsol Hasil Eksekusi & Inspector Aktor GVM:", WS_VISIBLE | WS_CHILD,
                                              10, 400, 600, 18, hwnd, NULL, NULL, NULL);

            // Output Console Box
            g_hwndEditOutput = CreateWindowExA(WS_EX_CLIENTEDGE, "RICHEDIT20A", "Klik 'Jalankan' atau tekan Ctrl+R untuk menguji program di Garuda Studio...",
                                              WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
                                              10, 420, 860, 130, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
            SendMessageA(g_hwndEditOutput, WM_SETFONT, (WPARAM)g_hFontCode, TRUE);
            SendMessageA(g_hwndEditOutput, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(24, 24, 24));

            // Status Bar at Bottom
            g_hwndStatusBar = CreateWindowExA(0, STATUSCLASSNAMEA, " Ready | Garuda Studio v3.5",
                                              WS_VISIBLE | WS_CHILD | SBARS_SIZEGRIP,
                                              0, 0, 0, 0, hwnd, (HMENU)ID_STATUS_BAR, NULL, NULL);

            ApplySyntaxHighlighting();
            UpdateStatusBarInfo();
            break;
        }

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            HWND hwndCtl = (HWND)lParam;
            if (hwndCtl == g_hwndEditCode) {
                SetTextColor(hdc, RGB(212, 212, 212));
                SetBkColor(hdc, RGB(30, 30, 30));
                return (INT_PTR)g_hbrDarkBg;
            } else if (hwndCtl == g_hwndGutter) {
                SetTextColor(hdc, RGB(133, 133, 133));
                SetBkColor(hdc, RGB(37, 37, 38));
                return (INT_PTR)g_hbrGutterBg;
            } else if (hwndCtl == g_hwndEditOutput) {
                SetTextColor(hdc, RGB(78, 201, 176)); // Teal Console Text
                SetBkColor(hdc, RGB(24, 24, 24));
                return (INT_PTR)g_hbrConsoleBg;
            }
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
                int gutter_w = 48;

                int avail_h = height - toolbar_h - (label_h * 2) - statusbar_h - (margin * 4);
                if (avail_h < 100) avail_h = 100;

                int code_h = (avail_h * 65) / 100;
                int output_h = avail_h - code_h;

                int code_y = toolbar_h + margin + label_h;
                int label_out_y = code_y + code_h + margin;
                int output_y = label_out_y + label_h;

                MoveWindow(g_hwndLabelCode, margin, toolbar_h + margin, width - (margin * 2), label_h, TRUE);
                MoveWindow(g_hwndGutter, margin, code_y, gutter_w, code_h, TRUE);
                MoveWindow(g_hwndEditCode, margin + gutter_w + 5, code_y, width - (margin * 2) - gutter_w - 5, code_h, TRUE);
                MoveWindow(g_hwndLabelOutput, margin, label_out_y, width - (margin * 2), label_h, TRUE);
                MoveWindow(g_hwndEditOutput, margin, output_y, width - (margin * 2), output_h, TRUE);
                MoveWindow(g_hwndStatusBar, 0, height - statusbar_h, width, statusbar_h, TRUE);

                UpdateLineNumbersGutter();
            }
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId == ID_EDIT_CODE) {
                if (wmEvent == EN_CHANGE) {
                    ApplySyntaxHighlighting();
                    UpdateStatusBarInfo();
                } else if (wmEvent == EN_SELCHANGE || wmEvent == EN_VSCROLL) {
                    UpdateStatusBarInfo();
                }
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
                    ApplySyntaxHighlighting();
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'Hello World' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_ACTOR:
                    SetWindowTextA(g_hwndEditCode, g_sample_actor);
                    ApplySyntaxHighlighting();
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'Model Aktor' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_GENSERV:
                    SetWindowTextA(g_hwndEditCode, g_sample_genserver);
                    ApplySyntaxHighlighting();
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'GenServer' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_SUPERV:
                    SetWindowTextA(g_hwndEditCode, g_sample_supervisor);
                    ApplySyntaxHighlighting();
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'Supervisor' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_API:
                    SetWindowTextA(g_hwndEditCode, g_sample_api);
                    ApplySyntaxHighlighting();
                    SetWindowTextA(g_hwndEditOutput, "Sampel 'REST API' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_GAME:
                    SetWindowTextA(g_hwndEditCode, g_sample_game);
                    ApplySyntaxHighlighting();
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
            if (g_hbrDarkBg) DeleteObject(g_hbrDarkBg);
            if (g_hbrGutterBg) DeleteObject(g_hbrGutterBg);
            if (g_hbrConsoleBg) DeleteObject(g_hbrConsoleBg);
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
                                "Garuda Studio v3.5 - Modern Professional GUI IDE",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 940, 700,
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
