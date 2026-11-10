#define _CRT_SECURE_NO_WARNINGS
// =====================================================
//  PYO[Pesan Yakin Online] (Simulasi Konsol)
//  Mengikuti flowchart: Pembelian_Tiket_Bioskop.drawio
//  Kompilasi : g++ -std=c++11 PYO.cpp -o PYO; .\PYO.exe
// =====================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cctype>
 
using namespace std;
 
// =========================
// 1. Konstanta & struktur data
// =========================
const int  BARIS             = 5;      // baris kursi A-E
const int  KOLOM             = 8;      // kolom kursi 1-8
const int  MAKS_TIKET        = 6;
const int  BATAS_KUNCI_DETIK = 600;    // kursi dikunci +-10 menit
const long BIAYA_LAYANAN     = 3000;   // per tiket
 
struct Akun   { string nama, email, password; };
struct Kota   { string nama; string cabang[2]; };
struct Film   { string judul; string durasi; long harga; };
struct ItemFnB{ string nama; long harga; };
struct PesanFnB { int idx; int jumlah; };
 
const string DAFTAR_JAM[4] = {"12:00", "14:30", "17:00", "19:30"};
const string NAMA_METODE[4] = {"E-Wallet", "Virtual Account (VA)", "Kartu Kredit/Debit", "QRIS"};
 
const Kota DAFTAR_KOTA[3] = {
    {"Surakarta", {"Solo Paragon Mall XXI", "Solo Grand Mall XXI"}},
    {"Yogyakarta", {"Malioboro XXI", "Jogja City Mall XXI"}},
    {"Semarang", {"Paragon Mall XXI", "Citra Land XXI"}}
};
 
const Film DAFTAR_FILM[5] = {
    {"Petualangan Nusantara", "120 menit", 45000},
    {"Malam di Kota Tua",     "105 menit", 40000},
    {"Cinta di Ujung Senja",  "98 menit",  40000},
    {"Galaksi Terakhir",      "135 menit", 55000},
    {"Si Kancil Kembali",     "90 menit",  35000}
};
 
const ItemFnB DAFTAR_FNB[5] = {
    {"Popcorn Besar",  35000},
    {"Popcorn Sedang", 28000},
    {"Minuman Soda",   18000},
    {"Air Mineral",    10000},
    {"Hot Dog",        25000}
};
 
// =========================
// 2. Fungsi bantu (input & format)
// =========================
string formatRupiah(long long nilai) {
    string s = to_string(nilai), hasil = "";
    int hitung = 0;
    for (int i = (int)s.size() - 1; i >= 0; i--) {
        hasil = s[i] + hasil;
        if (++hitung % 3 == 0 && i > 0) hasil = "." + hasil;
    }
    return "Rp " + hasil + ",00";
}
 
string bacaTeks(const string &prompt) {
    string s;
    do {
        cout << prompt;
        getline(cin, s);
        if (!cin) { cout << "\nInput berakhir.\n"; exit(0); }
    } while (s.empty());
    return s;
}
 
int bacaAngka(const string &prompt, int minimum, int maksimum) {
    while (true) {
        string s = bacaTeks(prompt);
        bool angka = true;
        for (size_t i = 0; i < s.size(); i++)
            if (!isdigit((unsigned char)s[i])) angka = false;
        if (angka && s.size() < 9) {
            int n = atoi(s.c_str());
            if (n >= minimum && n <= maksimum) return n;
        }
        cout << "  Input tidak valid! Masukkan angka " << minimum << "-" << maksimum << ".\n";
    }
}
 
string keHurufBesar(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = toupper((unsigned char)s[i]);
    return s;
}
 
string formatTanggal(time_t t) {
    char buf[20];
    strftime(buf, sizeof(buf), "%d-%m-%Y", localtime(&t));
    return string(buf);
}
 
string angkaEmpatDigit(int n) {
    ostringstream os;
    os << setw(4) << setfill('0') << n;
    return os.str();
}
 
// =========================
// 3. Fungsi akun (simpan di akun.txt, format: email;password;nama)
// =========================
vector<Akun> muatAkun() {
    vector<Akun> daftar;
    ifstream f("akun.txt");
    string baris;
    while (getline(f, baris)) {
        size_t p1 = baris.find(';');
        size_t p2 = baris.find(';', p1 + 1);
        if (p1 == string::npos || p2 == string::npos) continue;
        Akun a;
        a.email    = baris.substr(0, p1);
        a.password = baris.substr(p1 + 1, p2 - p1 - 1);
        a.nama     = baris.substr(p2 + 1);
        daftar.push_back(a);
    }
    if (daftar.empty()) {                       // akun contoh bila file belum ada
        Akun demo = {"Pengguna Demo", "demo@mail.com", "123456"};
        daftar.push_back(demo);
    }
    return daftar;
}
 
void simpanAkun(const vector<Akun> &daftar) {
    ofstream f("akun.txt");
    for (size_t i = 0; i < daftar.size(); i++)
        f << daftar[i].email << ';' << daftar[i].password << ';' << daftar[i].nama << '\n';
}
 
// Registrasi (data diri & OTP). Mengembalikan true bila berhasil.
bool registrasi(vector<Akun> &daftar) {
    cout << "\n--- REGISTRASI ---\n";
    Akun baru;
    baru.nama     = bacaTeks("Nama lengkap : ");
    baru.email    = bacaTeks("Email        : ");
    baru.password = bacaTeks("Password     : ");
    if (baru.nama.find(';') != string::npos || baru.email.find(';') != string::npos ||
        baru.password.find(';') != string::npos) {
        cout << "Karakter ';' tidak boleh dipakai.\n";
        return false;
    }
    for (size_t i = 0; i < daftar.size(); i++)
        if (daftar[i].email == baru.email) {
            cout << "Email sudah terdaftar.\n";
            return false;
        }
 
    int otp = 1000 + rand() % 9000;
    cout << "[SMS] Kode OTP Anda: " << otp << "\n";
    int masukan = bacaAngka("Masukkan kode OTP : ", 0, 99999999);
    if (masukan != otp) {
        cout << "OTP salah! Registrasi gagal.\n";
        return false;
    }
    daftar.push_back(baru);
    simpanAkun(daftar);
    cout << "Registrasi berhasil. Silakan login.\n";
    return true;
}
 
// Login, maksimal 3 percobaan. Mengembalikan true bila berhasil.
bool login(const vector<Akun> &daftar, Akun &pengguna) {
    cout << "\n--- LOGIN ---\n";
    for (int coba = 1; coba <= 3; coba++) {
        string email = bacaTeks("Email    : ");
        string pass  = bacaTeks("Password : ");
        for (size_t i = 0; i < daftar.size(); i++)
            if (daftar[i].email == email && daftar[i].password == pass) {
                pengguna = daftar[i];
                cout << "Login berhasil. Halo, " << pengguna.nama << "!\n";
                return true;
            }
        cout << "Email/password salah (" << coba << "/3).\n";
    }
    return false;
}
 
// =========================
// 4. Fungsi denah kursi
// =========================
void tampilDenah(bool terisi[BARIS][KOLOM]) {
    cout << "\n            [ LAYAR ]\n     ";
    for (int k = 0; k < KOLOM; k++) cout << k + 1 << ' ';
    cout << "\n";
    for (int b = 0; b < BARIS; b++) {
        cout << "  " << char('A' + b) << "  ";
        for (int k = 0; k < KOLOM; k++) cout << (terisi[b][k] ? 'X' : '.') << ' ';
        cout << "\n";
    }
    cout << "  (. = kosong, X = terisi/dipilih)\n";
}
 
// Ubah "A5" menjadi indeks baris & kolom. Mengembalikan false bila format salah.
bool parseKursi(string teks, int &baris, int &kolom) {
    teks = keHurufBesar(teks);
    if (teks.size() < 2) return false;
    baris = teks[0] - 'A';
    kolom = atoi(teks.substr(1).c_str()) - 1;
    return baris >= 0 && baris < BARIS && kolom >= 0 && kolom < KOLOM;
}
 
// Hitung nomor pesanan otomatis dari counter.txt
int ambilNomorPesanan() {
    int nomor = 1;
    ifstream in("counter.txt");
    if (in) in >> nomor;
    in.close();
    if (nomor < 1) nomor = 1;
    ofstream out("counter.txt");
    out << nomor + 1;
    return nomor;
}
 
// =========================
// PROGRAM UTAMA
// =========================
int main() {
    srand((unsigned)time(0));
    vector<Akun> daftarAkun = muatAkun();
    Akun pengguna;
 
    cout << "==============================\n";
    cout << "             PYO              \n";
    cout << "==============================\n";
 
    // =========================
    // 5. Sudah punya akun? -> Registrasi / Login
    // =========================
    bool sudahLogin = false;
    while (!sudahLogin) {
        int punya = bacaAngka("\nSudah punya akun? (1 = Ya, 2 = Belum) : ", 1, 2);
        if (punya == 2) {
            bool berhasil = false;
            while (!berhasil) berhasil = registrasi(daftarAkun);   // gagal -> ulangi
        }
        sudahLogin = login(daftarAkun, pengguna);
    }
 
    // =========================
    // 6. Pilih kota, cabang, film, tanggal
    // =========================
    cout << "\n--- PILIH KOTA ---\n";
    for (int i = 0; i < 3; i++) cout << i + 1 << ". " << DAFTAR_KOTA[i].nama << "\n";
    int kota = bacaAngka("Pilih kota : ", 1, 3) - 1;
 
    cout << "\n--- PILIH CABANG BIOSKOP ---\n";
    for (int i = 0; i < 2; i++) cout << i + 1 << ". " << DAFTAR_KOTA[kota].cabang[i] << "\n";
    int cabang = bacaAngka("Pilih cabang : ", 1, 2) - 1;
    string namaCabang = DAFTAR_KOTA[kota].cabang[cabang];
 
    cout << "\n--- PILIH FILM ---\n";
    for (int i = 0; i < 5; i++)
        cout << i + 1 << ". " << left << setw(24) << DAFTAR_FILM[i].judul
             << setw(11) << DAFTAR_FILM[i].durasi << formatRupiah(DAFTAR_FILM[i].harga) << "\n";
    int film = bacaAngka("Pilih film : ", 1, 5) - 1;
 
    cout << "\n--- PILIH TANGGAL ---\n";
    string tanggalPilihan[3];
    time_t sekarang = time(0);
    for (int i = 0; i < 3; i++) {
        tanggalPilihan[i] = formatTanggal(sekarang + i * 24 * 3600);
        cout << i + 1 << ". " << tanggalPilihan[i] << "\n";
    }
    string tanggal = tanggalPilihan[bacaAngka("Pilih tanggal : ", 1, 3) - 1];
 
    // =========================
    // 7. Pilih jam tayang (ulangi bila jadwal tidak tersedia)
    // =========================
    bool jadwalAda[4];
    for (int i = 0; i < 4; i++) jadwalAda[i] = (rand() % 4 != 0);   // simulasi ketersediaan
    jadwalAda[rand() % 4] = true;                                   // minimal 1 jadwal ada
 
    int jam = 0;
    bool tersedia = false;
    while (!tersedia) {
        cout << "\n--- PILIH JAM TAYANG ---\n";
        for (int i = 0; i < 4; i++)
            cout << i + 1 << ". " << DAFTAR_JAM[i] << (jadwalAda[i] ? "" : "  (PENUH)") << "\n";
        jam = bacaAngka("Pilih jam tayang : ", 1, 4) - 1;
        tersedia = jadwalAda[jam];
        if (!tersedia) cout << "Jadwal tidak tersedia, silakan pilih jam lain.\n";
    }
 
    // =========================
    // 8. Pilih jumlah tiket & kursi di denah
    // =========================
    int jumlahTiket = bacaAngka("\nPilih jumlah tiket (1-6) : ", 1, MAKS_TIKET);
 
    bool terisi[BARIS][KOLOM];
    for (int b = 0; b < BARIS; b++)
        for (int k = 0; k < KOLOM; k++) terisi[b][k] = (rand() % 4 == 0);   // simulasi kursi terisi
 
    vector<string> kursiDipilih;
    for (int n = 1; n <= jumlahTiket; n++) {
        bool kursiAda = false;
        while (!kursiAda) {
            tampilDenah(terisi);
            string masukan = bacaTeks("Pilih kursi ke-" + to_string(n) + " (contoh A5) : ");
            int b, k;
            if (!parseKursi(masukan, b, k)) {
                cout << "Format kursi tidak valid.\n";
            } else if (terisi[b][k]) {
                cout << "Kursi tidak tersedia, silakan pilih kursi lain.\n";
            } else {
                terisi[b][k] = true;                      // tandai terpilih
                kursiDipilih.push_back(keHurufBesar(masukan));
                kursiAda = true;
            }
        }
    }
 
    // =========================
    // 9. Kunci kursi sementara (+-10 menit)
    // =========================
    time_t waktuKunci = time(0);
    cout << "\nKursi dikunci sementara selama " << BATAS_KUNCI_DETIK / 60
         << " menit. Selesaikan pembayaran sebelum waktu habis.\n";
 
    // =========================
    // 10. Tambah F&B (opsional)
    // =========================
    vector<PesanFnB> pesanFnB;
    long totalFnB = 0;
    int tambah = bacaAngka("\nTambah F&B? (1 = Ya, 2 = Tidak) : ", 1, 2);
    if (tambah == 1) {
        cout << "\n--- MENU MAKANAN & MINUMAN ---\n";
        for (int i = 0; i < 5; i++)
            cout << i + 1 << ". " << left << setw(16) << DAFTAR_FNB[i].nama
                 << formatRupiah(DAFTAR_FNB[i].harga) << "\n";
        while (true) {
            int kode = bacaAngka("Masukkan kode menu (0 selesai) : ", 0, 5);
            if (kode == 0) break;
            int qty = bacaAngka("Masukkan jumlah : ", 1, 20);
            PesanFnB item = {kode - 1, qty};
            pesanFnB.push_back(item);
            totalFnB += (long)qty * DAFTAR_FNB[kode - 1].harga;
        }
    }
 
    // =========================
    // 11. Gunakan promo/voucher? (opsional)
    // =========================
    long hargaTiket    = (long)jumlahTiket * DAFTAR_FILM[film].harga;
    long biayaLayanan  = (long)jumlahTiket * BIAYA_LAYANAN;
    long subtotal      = hargaTiket + totalFnB + biayaLayanan;
    long potongan      = 0;
    string kodePromo   = "-";
 
    int pakaiPromo = bacaAngka("\nGunakan promo/voucher? (1 = Ya, 2 = Tidak) : ", 1, 2);
    if (pakaiPromo == 1) {
        string kode = keHurufBesar(bacaTeks("Masukkan kode promo : "));
        if (kode == "HEMAT10") {
            potongan = (hargaTiket + totalFnB) * 10 / 100;
            kodePromo = kode;
            cout << "Promo HEMAT10 berhasil (diskon 10%).\n";
        } else if (kode == "FILM15K") {
            potongan = 15000;
            kodePromo = kode;
            cout << "Promo FILM15K berhasil (potongan Rp 15.000).\n";
        } else {
            cout << "Kode promo tidak valid, lanjut tanpa promo.\n";
        }
        if (potongan > subtotal) potongan = subtotal;
    }
    long totalBayar = subtotal - potongan;
 
    // =========================
    // 12. Cek ringkasan pesanan & harga total
    // =========================
    cout << "\n==============================\n";
    cout << "        RINGKASAN PESANAN\n";
    cout << "==============================\n";
    cout << "Pemesan  : " << pengguna.nama << "\n";
    cout << "Bioskop  : " << namaCabang << ", " << DAFTAR_KOTA[kota].nama << "\n";
    cout << "Film     : " << DAFTAR_FILM[film].judul << "\n";
    cout << "Jadwal   : " << tanggal << " pukul " << DAFTAR_JAM[jam] << "\n";
    cout << "Kursi    : ";
    for (size_t i = 0; i < kursiDipilih.size(); i++) cout << kursiDipilih[i] << ' ';
    cout << "\n------------------------------\n";
    cout << left << setw(22) << "Tiket (" + to_string(jumlahTiket) + "x)" << formatRupiah(hargaTiket) << "\n";
    for (size_t i = 0; i < pesanFnB.size(); i++)
        cout << left << setw(22)
             << (DAFTAR_FNB[pesanFnB[i].idx].nama + " x" + to_string(pesanFnB[i].jumlah))
             << formatRupiah((long)pesanFnB[i].jumlah * DAFTAR_FNB[pesanFnB[i].idx].harga) << "\n";
    cout << left << setw(22) << "Biaya layanan" << formatRupiah(biayaLayanan) << "\n";
    cout << left << setw(22) << ("Promo (" + kodePromo + ")") << "-" << formatRupiah(potongan) << "\n";
    cout << "------------------------------\n";
    cout << left << setw(22) << "TOTAL BAYAR" << formatRupiah(totalBayar) << "\n";
 
    // =========================
    // 13. Pilih metode & lakukan pembayaran
    // =========================
    bool lunas = false, batal = false;
    string metode = "-", refID = "-";
    while (!lunas && !batal) {
        cout << "\n--- PILIH METODE PEMBAYARAN ---\n";
        for (int i = 0; i < 4; i++) cout << i + 1 << ". " << NAMA_METODE[i] << "\n";
        int m = bacaAngka("Pilih metode pembayaran : ", 1, 4) - 1;
        metode = NAMA_METODE[m];
        refID  = "PAY" + string(m == 0 ? "EWL" : m == 1 ? "VA" : m == 2 ? "CC" : "QRS")
                 + angkaEmpatDigit(rand() % 10000);
 
        cout << "\nMetode   : " << metode << "\n";
        cout << "Ref ID   : " << refID << "\n";
        cout << "Nominal  : " << formatRupiah(totalBayar) << "\n";
        cout << "Silakan lakukan pembayaran melalui " << metode << ".\n";
 
        // Pembayaran berhasil?
        int hasil = bacaAngka("Hasil pembayaran (1 = Berhasil, 2 = Gagal/Belum) : ", 1, 2);
        if (hasil == 1) {
            lunas = true;
        } else {
            // Batas waktu habis?
            if (difftime(time(0), waktuKunci) >= BATAS_KUNCI_DETIK) {
                batal = true;
            } else {
                cout << "Pembayaran belum berhasil, waktu belum habis. Silakan pilih metode lagi.\n";
            }
        }
    }
 
    if (batal) {
        cout << "\nBatas waktu pembayaran habis.\n";
        cout << "Pesanan dibatalkan & kursi dilepas.\n";
        cout << "\n=== SELESAI ===\n";
        return 0;
    }
 
    // =========================
    // 14. Sistem membuat e-ticket & kode QR, lalu kirim
    // =========================
    int nomorPesanan = ambilNomorPesanan();
    string kodeQR = "QR-" + angkaEmpatDigit(nomorPesanan) + "-" + angkaEmpatDigit(rand() % 10000);
    string namaFile = "TIKET" + angkaEmpatDigit(nomorPesanan) + ".txt";
 
    ostringstream tiket;
    tiket << "================ E-TICKET ================\n";
    tiket << "No. Pesanan : " << angkaEmpatDigit(nomorPesanan) << "\n";
    tiket << "Ref Bayar   : " << refID << " (" << metode << ")\n";
    tiket << "Pemesan     : " << pengguna.nama << "\n";
    tiket << "Bioskop     : " << namaCabang << ", " << DAFTAR_KOTA[kota].nama << "\n";
    tiket << "Film        : " << DAFTAR_FILM[film].judul << "\n";
    tiket << "Jadwal      : " << tanggal << " pukul " << DAFTAR_JAM[jam] << "\n";
    tiket << "Kursi       : ";
    for (size_t i = 0; i < kursiDipilih.size(); i++) tiket << kursiDipilih[i] << ' ';
    tiket << "\nTotal Bayar : " << formatRupiah(totalBayar) << "\n";
    tiket << "Status      : LUNAS\n";
    tiket << "Kode QR     : " << kodeQR << "\n";
    tiket << "==========================================\n";
 
    cout << "\nPembayaran berhasil!\n\n" << tiket.str();
    ofstream fileTiket(namaFile.c_str());
    fileTiket << tiket.str();
    fileTiket.close();
    cout << "E-ticket dikirim ke " << pengguna.email << " dan disimpan di " << namaFile << "\n";
 
    // =========================
    // 15. Datang ke bioskop & scan QR di gate
    // =========================
    cout << "\nDatang ke bioskop sesuai jadwal (" << tanggal << " pukul " << DAFTAR_JAM[jam] << ").\n";
    cout << "Tekan Enter jika sudah tiba di gate...";
    string tunggu;
    getline(cin, tunggu);
 
    bool qrValid = false;
    while (!qrValid) {
        string scan = bacaTeks("Scan QR di gate/petugas (ketik kode QR) : ");
        qrValid = (keHurufBesar(scan) == kodeQR);
        if (!qrValid) {
            cout << "QR tidak valid!\n";
            cout << "Hubungi petugas/customer service, lalu lakukan verifikasi ulang.\n";
        }
    }
 
    // =========================
    // 16. Masuk studio & menonton film
    // =========================
    cout << "\nQR valid. Silakan masuk studio dan selamat menonton \"" << DAFTAR_FILM[film].judul << "\"!\n";
    cout << "\n=== SELESAI ===\n";
    return 0;
}
 
