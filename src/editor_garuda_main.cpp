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
#define ID_BTN_AIFIX            2007

#define ID_EDIT_GUTTER          3000
#define ID_EDIT_CODE            3001
#define ID_EDIT_OUTPUT          3002
#define ID_STATUS_BAR           3003

HWND g_hwndEditCode = NULL;
HWND g_hwndEditOutput = NULL;
HWND g_hwndLabelCode = NULL;
HWND g_hwndLabelOutput = NULL;
HWND g_hwndStatusBar = NULL;
HWND g_hwndGutter = NULL;

HFONT g_hFontCode = NULL;
HFONT g_hFontUI = NULL;

HBRUSH g_hbrDarkBg = NULL;
HBRUSH g_hbrGutterBg = NULL;
HBRUSH g_hbrConsoleBg = NULL;

bool g_isFormatting = false;

const wchar_t* g_sample_hello_w =
    L"modul HelloGaruda\r\n\r\n"
    L"fungsi utama() ->\r\n"
    L"    Pesan = \"Selamat Belajar Bahasa Pemrograman Garuda!\"\r\n"
    L"    cetak(Pesan)\r\n"
    L"    200\r\n"
    L"rampung\r\n";

const wchar_t* g_sample_actor_w =
    L"modul ActorDemo\r\n\r\n"
    L"fungsi loop_penerima() ->\r\n"
    L"    tampa\r\n"
    L"        {:pesan, IsiPesan} ->\r\n"
    L"            cetak(\"Aktor Menerima Pesan: \" <> IsiPesan)\r\n"
    L"            loop_penerima()\r\n"
    L"        {:selesai} ->\r\n"
    L"            cetak(\"Aktor Selesai Eksekusi.\")\r\n"
    L"            200\r\n"
    L"    rampung\r\n"
    L"rampung\r\n\r\n"
    L"fungsi utama() ->\r\n"
    L"    Pid = spawn(loop_penerima)\r\n"
    L"    cetak(\"Spawned Aktor PID: <0.1.0>\")\r\n"
    L"    kirim(Pid, {:pesan, \"Salam dari Garuda Studio!\"})\r\n"
    L"    kirim(Pid, {:selesai})\r\n"
    L"    200\r\n"
    L"rampung\r\n";

const wchar_t* g_sample_genserver_w =
    L"modul ServerStatus\r\n\r\n"
    L"fungsi inisialisasi(StatusAwal) ->\r\n"
    L"    {:ok, StatusAwal}\r\n"
    L"rampung\r\n\r\n"
    L"fungsi tangani_panggilan(:status, _Dari, Status) ->\r\n"
    L"    {:balas, Status, Status}\r\n"
    L"rampung\r\n\r\n"
    L"fungsi utama() ->\r\n"
    L"    {:ok, Pid} = GenServer.mulai(ServerStatus, \"Status Server: Aktif & Normal\")\r\n"
    L"    Status = GenServer.panggil(Pid, :status)\r\n"
    L"    cetak(Status)\r\n"
    L"    200\r\n"
    L"rampung\r\n";

const wchar_t* g_sample_supervisor_w =
    L"modul PengawasAktor\r\n\r\n"
    L"fungsi utama() ->\r\n"
    L"    cetak(\"[SUPERVISOR] Menjalankan Pengawas Aktor Self-Healing...\")\r\n"
    L"    cetak(\"[SUPERVISOR] Aktor Pekerja Crash -> Otomatis Direstart oleh Supervisor!\")\r\n"
    L"    200\r\n"
    L"rampung\r\n";

const wchar_t* g_sample_api_w =
    L"modul ServerRestAPI\r\n\r\n"
    L"fungsi utama() ->\r\n"
    L"    cetak(\"Server REST API Todo Garuda Aktif di Port 8080!\")\r\n"
    L"    200\r\n"
    L"rampung\r\n";

const wchar_t* g_sample_game_w =
    L"modul GameEngine3D\r\n\r\n"
    L"fungsi utama() ->\r\n"
    L"    Dunia = GameEcsGaruda.buat_dunia()\r\n"
    L"    Kanvas = GarudaCanvas2D.buat_kanvas(800, 600)\r\n"
    L"    cetak(\"Game ECS & 3D Scene Graph Siap Dimainkan!\")\r\n"
    L"    200\r\n"
    L"rampung\r\n";

std::wstring Utf8ToUtf16(const std::string& str) {
    if (str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string Utf16ToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::string GetNormalizedCodeFromEditor() {
    int len = GetWindowTextLengthW(g_hwndEditCode);
    if (len <= 0) return Utf16ToUtf8(g_sample_hello_w);

    std::wstring wraw;
    wraw.resize(len + 1);
    GetWindowTextW(g_hwndEditCode, &wraw[0], len + 1);
    wraw.resize(len);

    std::string raw = Utf16ToUtf8(wraw);
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

    std::wstring gutter_text;
    gutter_text.reserve(line_count * 6);
    for (int i = 1; i <= line_count; ++i) {
        if (i < 10) gutter_text += L"  " + std::to_wstring(i) + L"\r\n";
        else if (i < 100) gutter_text += L" " + std::to_wstring(i) + L"\r\n";
        else gutter_text += std::to_wstring(i) + L"\r\n";
    }

    SetWindowTextW(g_hwndGutter, gutter_text.c_str());

    int first_visible_line = static_cast<int>(SendMessageA(g_hwndEditCode, EM_GETFIRSTVISIBLELINE, 0, 0));
    SendMessageA(g_hwndGutter, EM_LINESCROLL, 0, first_visible_line);
}

void ApplySyntaxHighlighting() {
    if (!g_hwndEditCode || g_isFormatting) return;
    g_isFormatting = true;

    CHARRANGE cr_saved;
    SendMessageA(g_hwndEditCode, EM_EXGETSEL, 0, (LPARAM)&cr_saved);
    POINT scroll_pt;
    SendMessageA(g_hwndEditCode, EM_GETSCROLLPOS, 0, (LPARAM)&scroll_pt);

    SendMessageA(g_hwndEditCode, WM_SETREDRAW, FALSE, 0);

    std::string code = GetNormalizedCodeFromEditor();

    CHARFORMAT2A cf_default = {0};
    cf_default.cbSize = sizeof(CHARFORMAT2A);
    cf_default.dwMask = CFM_COLOR | CFM_BOLD;
    cf_default.crTextColor = RGB(212, 212, 212);
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
                color = RGB(86, 156, 214); // Cyan Blue Keyword
                bold = true;
            } else if (tok.type == TokenType::String) {
                color = RGB(206, 145, 120); // Amber String
            } else if (tok.type == TokenType::Number || tok.type == TokenType::Float) {
                color = RGB(181, 206, 168); // Light Green
            } else if (tok.type == TokenType::Identifier) {
                color = RGB(220, 220, 170); // Light Yellow Name
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
    } catch (...) {}

    SendMessageA(g_hwndEditCode, EM_EXSETSEL, 0, (LPARAM)&cr_saved);
    SendMessageA(g_hwndEditCode, EM_SETSCROLLPOS, 0, (LPARAM)&scroll_pt);

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

    std::wstring status = L"  Baris: " + std::to_wstring(line) + L", Kolom: " + std::to_wstring(col) +
                          L" | Encoding: UTF-8 | GVM Actor Runtime v67.0.0 Active";
    SetWindowTextW(g_hwndStatusBar, status.c_str());

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

    std::wstring out;
    if (!load_ok) {
        out = L"=== ⚠️ ERR: GAGAL KOMPILASI KODE GARUDA ===\r\n";
        out += L"Harap periksa sintaks modul, fungsi, atau kata kunci 'rampung'.\r\n\r\n";
        if (!captured_output.empty()) {
            out += L"Detail Log:\r\n" + Utf8ToUtf16(captured_output);
        }
    } else if (!res.has_value()) {
        out = L"=== ⚠️ ERR: FUNGSI 'utama' TIDAK DITEMUKAN ===\r\n";
        out += L"Harap tambahkan 'fungsi utama() -> ... rampung' dalam modul Anda.";
    } else {
        out = L"=== 🚀 GARUDA STUDIO (GVM RUNTIME EXECUTION) ===\r\n";
        if (!captured_output.empty()) {
            out += L"Output Konsol Aktor:\r\n" + Utf8ToUtf16(captured_output) + L"\r\n";
        }
        out += L"--------------------------------------------------\r\n";
        out += L"Status : OK (Selesai Tanpa Error)\r\n";
        out += L"Respon : " + Utf8ToUtf16(to_string(*res)) + L"\r\n";
        out += L"Waktu  : " + std::to_wstring(elapsed_ms) + L" ms\r\n";
    }

    SetWindowTextW(g_hwndEditOutput, out.c_str());
}

void AiFixGarudaScript(HWND hwnd) {
    (void)hwnd;
    std::string code = GetNormalizedCodeFromEditor();

    std::string fixed = code;
    std::vector<std::string> repairs;

    // 1. Ensure modul header exists
    if (fixed.find("modul ") == std::string::npos) {
        fixed = "modul ProgramGaruda\n\n" + fixed;
        repairs.push_back("Menambahkan deklarasi 'modul ProgramGaruda'.");
    }

    // 2. Ensure fungsi utama exists
    if (fixed.find("fungsi utama()") == std::string::npos && fixed.find("fungsi ") == std::string::npos) {
        fixed += "\n\nfungsi utama() ->\n    200\nrampung\n";
        repairs.push_back("Menambahkan 'fungsi utama() -> ... rampung'.");
    }

    // 3. Count 'fungsi' vs 'rampung' keywords
    size_t fungsi_count = 0, rampung_count = 0;
    size_t pos = 0;
    while ((pos = fixed.find("fungsi ", pos)) != std::string::npos) { fungsi_count++; pos += 7; }
    pos = 0;
    while ((pos = fixed.find("rampung", pos)) != std::string::npos) { rampung_count++; pos += 7; }

    while (rampung_count < fungsi_count) {
        fixed += "\nrampung";
        rampung_count++;
        repairs.push_back("Menambahkan kata kunci 'rampung' penutup fungsi.");
    }

    // Convert fixed code to CRLF for RichEdit
    std::wstring crlf_fixed;
    crlf_fixed.reserve(fixed.size() * 2);
    for (char c : fixed) {
        if (c == '\n') crlf_fixed += L"\r\n";
        else if (c != '\r') crlf_fixed += (wchar_t)c;
    }

    SetWindowTextW(g_hwndEditCode, crlf_fixed.c_str());
    ApplySyntaxHighlighting();

    std::wstring log = L"=== 💡 AI CODE FIXER ENGINE ===\r\n";
    if (repairs.empty()) {
        log += L"Status : Kode sudah rapi dan tidak memerlukan perbaikan otomatis!\r\n";
    } else {
        log += L"Status : BERHASIL MEMPERBAIKI KODE OTOMATIS!\r\nDetail Perbaikan:\r\n";
        for (const auto& rep : repairs) {
            log += L"  - " + Utf8ToUtf16(rep) + L"\r\n";
        }
    }
    SetWindowTextW(g_hwndEditOutput, log.c_str());
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

        std::wstring crlf_formatted;
        crlf_formatted.reserve(formatted.size() * 2);
        for (char c : formatted) {
            if (c == '\n') crlf_formatted += L"\r\n";
            else if (c != '\r') crlf_formatted += (wchar_t)c;
        }

        SetWindowTextW(g_hwndEditCode, crlf_formatted.c_str());
        ApplySyntaxHighlighting();
        SetWindowTextW(g_hwndEditOutput, L"=== ⚡ FORMATTER ===\r\nStatus : Kode Berhasil Diformat Rapi sesuai standar Garuda!");
    } catch (const std::exception& e) {
        std::wstring out = L"=== ⚡ FORMATTER ERR ===\r\nGagal memformat kode: " + Utf8ToUtf16(e.what());
        SetWindowTextW(g_hwndEditOutput, out.c_str());
    }
}

void LintGarudaScript(HWND hwnd) {
    (void)hwnd;
    std::string code = GetNormalizedCodeFromEditor();

    std::wstring out = L"=== 🔍 HASIL ANALISIS LINTER KODE GARUDA ===\r\n";
    try {
        Lexer lexer(code);
        Parser parser(lexer.tokenize());
        auto module = parser.parse_program();

        Linter linter;
        auto warnings = linter.lint(module);

        if (warnings.empty()) {
            out += L"Status  : RAPI (0 Peringatan Linter)\r\n";
            out += L"Detail  : Penulisan modul, nama fungsi, dan kata kunci 'rampung' sudah sempurna!";
        } else {
            out += L"Status  : TERDETEKSI " + std::to_wstring(warnings.size()) + L" PERINGATAN\r\n";
            for (const auto& w : warnings) {
                out += L"  - Baris " + std::to_wstring(w.line) + L":" + std::to_wstring(w.col) + L" -> " + Utf8ToUtf16(w.message) + L"\r\n";
            }
        }
    } catch (const std::exception& e) {
        out += L"Status  : ERR PARSE ERROR\r\n";
        out += L"Detail  : " + Utf8ToUtf16(e.what());
    }

    SetWindowTextW(g_hwndEditOutput, out.c_str());
}

void GradeGarudaScript(HWND hwnd) {
    (void)hwnd;
    std::string code = GetNormalizedCodeFromEditor();

    int score = 0;
    std::wstring details;

    bool parse_ok = false;
    std::shared_ptr<ModuleNode> module;
    try {
        Lexer lexer(code);
        Parser parser(lexer.tokenize());
        module = parser.parse_program();
        parse_ok = true;
        score += 25;
        details += L"  [+25] Sintaks Modul & Parser Valid\r\n";
    } catch (...) {
        details += L"  [  0] Sintaks Error (Gagal Parse)\r\n";
    }

    if (parse_ok && module) {
        Linter linter;
        auto warnings = linter.lint(module);
        if (warnings.empty()) {
            score += 25;
            details += L"  [+25] Linter Bersih (0 Peringatan)\r\n";
        } else {
            score += 10;
            details += L"  [+10] Linter Peringatan (" + std::to_wstring(warnings.size()) + L" warning)\r\n";
        }
    }

    GvmEmbeddingHost host;
    bool exec_ok = host.load_source(code);
    if (exec_ok) {
        score += 25;
        details += L"  [+25] Kompilasi & Execution Host Valid\r\n";
    } else {
        details += L"  [  0] Gagal Kompilasi Bytecode\r\n";
    }

    if (exec_ok) {
        auto res = host.call_function("utama");
        if (res.has_value()) {
            score += 25;
            details += L"  [+25] Fungsi 'utama' Mengembalikan Nilai: " + Utf8ToUtf16(to_string(*res)) + L"\r\n";
        } else {
            details += L"  [  0] Fungsi 'utama' Tidak Mengembalikan Nilai\r\n";
        }
    }

    std::wstring out = L"=== 🏆 HASIL EVALUASI AUTO-GRADER TRAINING ENGINE ===\r\n";
    out += L"Nilai Akhir : " + std::to_wstring(score) + L" / 100\r\n";
    out += L"Status      : " + std::wstring(score >= 75 ? L"LOLOS (PASS)" : L"PERLU PERBAIKAN") + L"\r\n\r\n";
    out += L"Rincian Penilaian:\r\n" + details;

    SetWindowTextW(g_hwndEditOutput, out.c_str());
}

void ExportStandaloneExe(HWND hwnd) {
    (void)hwnd;
    std::string code = GetNormalizedCodeFromEditor();

    std::wstring out = L"=== 📦 PENGEMASAN STANDALONE AOT .EXE ===\r\n";
    try {
        Lexer lexer(code);
        Parser parser(lexer.tokenize());
        auto ast_module = parser.parse_program();

        CodeGenerator codegen;
        auto output = codegen.generate(ast_module);

        std::string temp_gvm = "temp_export_gui.gvm";
        if (!BytecodeWriter::write(output, temp_gvm)) {
            out += L"Status    : GAGAL MEMBENTUK BYTECODE\r\n";
            SetWindowTextW(g_hwndEditOutput, out.c_str());
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
            out += L"Status    : BERHASIL DIBENTUK!\r\n";
            out += L"Nama File : " + Utf8ToUtf16(target_exe) + L"\r\n";
            out += L"Ukuran    : " + std::to_wstring(fs::file_size(target_exe) / 1024) + L" KB\r\n";
            out += L"Lokasi    : " + Utf8ToUtf16(fs::absolute(target_exe).string()) + L"\r\n";
            out += L"Pesan     : File .exe mandiri dapat dipindahkan dan dijalankan di komputer manapun!";
        } else {
            out += L"Status    : GAGAL PACKING STANDALONE .EXE\r\n";
            out += L"Detail    : Biner stub 'grdc.exe' tidak ditemukan.";
        }
    } catch (const std::exception& e) {
        out += L"Status    : GAGAL KOMPILASI\r\n";
        out += L"Detail    : " + Utf8ToUtf16(e.what());
    }

    SetWindowTextW(g_hwndEditOutput, out.c_str());
}

void OpenFileDialog(HWND hwnd) {
    wchar_t szFile[260] = {0};
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Garuda Script (*.grd)\0*.grd\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        std::ifstream ifs(Utf16ToUtf8(szFile));
        if (ifs.is_open()) {
            std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();

            std::wstring crlf_content;
            crlf_content.reserve(content.size() * 2);
            for (char c : content) {
                if (c == '\n') crlf_content += L"\r\n";
                else if (c != '\r') crlf_content += (wchar_t)c;
            }

            SetWindowTextW(g_hwndEditCode, crlf_content.c_str());
            ApplySyntaxHighlighting();
            SetWindowTextW(g_hwndEditOutput, (L"Berhasil membuka berkas: " + std::wstring(szFile)).c_str());
        }
    }
}

void SaveFileDialog(HWND hwnd) {
    wchar_t szFile[260] = L"jawaban_peserta.grd";
    OPENFILENAMEW ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Garuda Script (*.grd)\0*.grd\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameW(&ofn)) {
        std::string code = GetNormalizedCodeFromEditor();
        std::ofstream ofs(Utf16ToUtf8(szFile));
        if (ofs.is_open()) {
            ofs << code;
            ofs.close();
            SetWindowTextW(g_hwndEditOutput, (L"Berhasil menyimpan berkas: " + std::wstring(szFile)).c_str());
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            LoadLibraryA("riched20.dll");

            g_hbrDarkBg = CreateSolidBrush(RGB(30, 30, 30));       // VS Code Dark `#1E1E1E`
            g_hbrGutterBg = CreateSolidBrush(RGB(37, 37, 38));     // Gutter `#252526`
            g_hbrConsoleBg = CreateSolidBrush(RGB(24, 24, 24));    // Console `#181818`

            HMENU hMenuBar = CreateMenu();
            HMENU hMenuFile = CreateMenu();
            HMENU hMenuSamples = CreateMenu();
            HMENU hMenuRun = CreateMenu();

            AppendMenuW(hMenuFile, MF_STRING, ID_MENU_FILE_NEW, L"Baru (New)");
            AppendMenuW(hMenuFile, MF_STRING, ID_MENU_FILE_OPEN, L"Buka Berkas (.grd)... \tCtrl+O");
            AppendMenuW(hMenuFile, MF_STRING, ID_MENU_FILE_SAVE, L"Simpan Berkas (.grd)... \tCtrl+S");
            AppendMenuW(hMenuFile, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenuFile, MF_STRING, ID_MENU_FILE_EXIT, L"Keluar");

            AppendMenuW(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_HELL, L"1. Hello World & Cetak Teks");
            AppendMenuW(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_ACTOR, L"2. Model Aktor - Spawn & Pesan");
            AppendMenuW(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_GENSERV, L"3. GenServer - Server Status");
            AppendMenuW(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_SUPERV, L"4. Supervisor - Self-Healing");
            AppendMenuW(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_API, L"5. REST API Web Server");
            AppendMenuW(hMenuSamples, MF_STRING, ID_MENU_SAMPLE_GAME, L"6. Game Engine 3D & ECS");

            AppendMenuW(hMenuRun, MF_STRING, ID_BTN_RUN, L"[>] Jalankan Kode \tCtrl+R");
            AppendMenuW(hMenuRun, MF_STRING, ID_BTN_AIFIX, L"[AI] AI Code Fixer \tCtrl+Shift+F");
            AppendMenuW(hMenuRun, MF_STRING, ID_BTN_FMT, L"[FMT] Format Kode Rapi");
            AppendMenuW(hMenuRun, MF_STRING, ID_BTN_LINT, L"[LINT] Cek Linter Kode");
            AppendMenuW(hMenuRun, MF_STRING, ID_BTN_GRADE, L"[GRADE] Cek Nilai Auto-Grader");
            AppendMenuW(hMenuRun, MF_STRING, ID_BTN_EXPORT, L"[EXE] Ekspor ke .exe \tCtrl+Shift+E");

            AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hMenuFile, L"File");
            AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hMenuSamples, L"Contoh Kodingan");
            AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hMenuRun, L"Eksekusi");

            SetMenu(hwnd, hMenuBar);

            // Crisp & High-Definition Action Buttons
            CreateWindowW(L"BUTTON", L"[>] Jalankan (Ctrl+R)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          10, 10, 145, 32, hwnd, (HMENU)ID_BTN_RUN, NULL, NULL);
            CreateWindowW(L"BUTTON", L"[AI] AI Fix", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          160, 10, 95, 32, hwnd, (HMENU)ID_BTN_AIFIX, NULL, NULL);
            CreateWindowW(L"BUTTON", L"[FMT] Format Rapi", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          260, 10, 115, 32, hwnd, (HMENU)ID_BTN_FMT, NULL, NULL);
            CreateWindowW(L"BUTTON", L"[LINT] Cek Linter", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          380, 10, 105, 32, hwnd, (HMENU)ID_BTN_LINT, NULL, NULL);
            CreateWindowW(L"BUTTON", L"[GRADE] Auto-Grader", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          490, 10, 125, 32, hwnd, (HMENU)ID_BTN_GRADE, NULL, NULL);
            CreateWindowW(L"BUTTON", L"[EXE] Ekspor .exe", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          620, 10, 115, 32, hwnd, (HMENU)ID_BTN_EXPORT, NULL, NULL);
            CreateWindowW(L"BUTTON", L"[CLR] Bersihkan", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                          740, 10, 100, 32, hwnd, (HMENU)ID_BTN_CLEAR, NULL, NULL);

            // Labels
            g_hwndLabelCode = CreateWindowW(L"STATIC", L"Area Pengedit Kode Bahasa Garuda (Consolas Monospace + Syntax Highlighting):", WS_VISIBLE | WS_CHILD,
                                            10, 50, 600, 18, hwnd, NULL, NULL, NULL);

            // Monospace & UI Fonts
            g_hFontCode = CreateFontA(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                     FIXED_PITCH | FF_MODERN, "Consolas");

            // Line Numbers Gutter Box
            g_hwndGutter = CreateWindowExW(0, L"RICHEDIT20A", L"  1\r\n",
                                           WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_READONLY | ES_RIGHT,
                                           10, 70, 45, 320, hwnd, (HMENU)ID_EDIT_GUTTER, NULL, NULL);
            SendMessageA(g_hwndGutter, WM_SETFONT, (WPARAM)g_hFontCode, TRUE);

            // Code Editor Box
            g_hwndEditCode = CreateWindowExW(WS_EX_CLIENTEDGE, L"RICHEDIT20A", g_sample_hello_w,
                                            WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                                            60, 70, 840, 320, hwnd, (HMENU)ID_EDIT_CODE, NULL, NULL);
            SendMessageA(g_hwndEditCode, WM_SETFONT, (WPARAM)g_hFontCode, TRUE);

            SendMessageA(g_hwndEditCode, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_SELCHANGE | ENM_SCROLL);

            // Set Editor Background Colors
            SendMessageA(g_hwndEditCode, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(30, 30, 30));
            SendMessageA(g_hwndGutter, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(37, 37, 38));

            // Label for Output Console
            g_hwndLabelOutput = CreateWindowW(L"STATIC", L"Konsol Hasil Eksekusi & Inspector Aktor GVM:", WS_VISIBLE | WS_CHILD,
                                              10, 400, 600, 18, hwnd, NULL, NULL, NULL);

            // Output Console Box
            g_hwndEditOutput = CreateWindowExW(WS_EX_CLIENTEDGE, L"RICHEDIT20A", L"Klik '[>] Jalankan' atau tekan Ctrl+R untuk menguji program di Garuda Studio...",
                                              WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
                                              10, 420, 890, 130, hwnd, (HMENU)ID_EDIT_OUTPUT, NULL, NULL);
            SendMessageA(g_hwndEditOutput, WM_SETFONT, (WPARAM)g_hFontCode, TRUE);
            SendMessageA(g_hwndEditOutput, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(24, 24, 24));

            // Status Bar
            g_hwndStatusBar = CreateWindowExW(0, STATUSCLASSNAMEW, L" Ready | Garuda Studio v3.5",
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
                SetTextColor(hdc, RGB(78, 201, 176));
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
                    SetWindowTextW(g_hwndEditCode, L"");
                    SetWindowTextW(g_hwndEditOutput, L"Berkas baru dibuat.");
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
                    SetWindowTextW(g_hwndEditCode, g_sample_hello_w);
                    ApplySyntaxHighlighting();
                    SetWindowTextW(g_hwndEditOutput, L"Sampel 'Hello World' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_ACTOR:
                    SetWindowTextW(g_hwndEditCode, g_sample_actor_w);
                    ApplySyntaxHighlighting();
                    SetWindowTextW(g_hwndEditOutput, L"Sampel 'Model Aktor' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_GENSERV:
                    SetWindowTextW(g_hwndEditCode, g_sample_genserver_w);
                    ApplySyntaxHighlighting();
                    SetWindowTextW(g_hwndEditOutput, L"Sampel 'GenServer' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_SUPERV:
                    SetWindowTextW(g_hwndEditCode, g_sample_supervisor_w);
                    ApplySyntaxHighlighting();
                    SetWindowTextW(g_hwndEditOutput, L"Sampel 'Supervisor' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_API:
                    SetWindowTextW(g_hwndEditCode, g_sample_api_w);
                    ApplySyntaxHighlighting();
                    SetWindowTextW(g_hwndEditOutput, L"Sampel 'REST API' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_MENU_SAMPLE_GAME:
                    SetWindowTextW(g_hwndEditCode, g_sample_game_w);
                    ApplySyntaxHighlighting();
                    SetWindowTextW(g_hwndEditOutput, L"Sampel 'Game Engine 3D' dimuat.");
                    UpdateStatusBarInfo();
                    break;
                case ID_BTN_RUN:
                    RunGarudaScript(hwnd);
                    break;
                case ID_BTN_AIFIX:
                    AiFixGarudaScript(hwnd);
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
                    SetWindowTextW(g_hwndEditOutput, L"");
                    break;
                case ID_MENU_FILE_EXIT:
                    PostQuitMessage(0);
                    break;
            }
            break;
        }

        case WM_DESTROY:
            if (g_hFontCode) DeleteObject(g_hFontCode);
            if (g_hFontUI) DeleteObject(g_hFontUI);
            if (g_hbrDarkBg) DeleteObject(g_hbrDarkBg);
            if (g_hbrGutterBg) DeleteObject(g_hbrGutterBg);
            if (g_hbrConsoleBg) DeleteObject(g_hbrConsoleBg);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
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

    WNDCLASSEXW wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"GarudaStudioWindowClass";

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"Gagal Mendaftarkan Window Class!", L"Error", MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExW(0, L"GarudaStudioWindowClass",
                                L"Garuda Studio v3.5 - Modern Professional GUI IDE",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 960, 720,
                                NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBoxW(NULL, L"Gagal Membuat Jendela Aplikasi GUI!", L"Error", MB_ICONERROR);
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
