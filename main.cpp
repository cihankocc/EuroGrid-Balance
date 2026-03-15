
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <iomanip>
using namespace std;
   // hesaplamaları tutan graph düğümğ
struct Ulke {
    int id;
    char ad[40];
    double uretim, tuketim, cezaKatsayisi, satisFiyati;
    double netEnerji, maliyet, karZarar;
    bool cezaOdediMi;
    double alinanMiktar;
    int kaynaktan;
};
   //sistemi yöneten ana class
class EnerjiSistemi {
private:
    int ulkeSayisi;
    Ulke* ulkeler;
    double** mesafe;
    // büyükten küçüğe rekürsif quickshort
    void quickSort(double dizi[], int idx[], int sol, int sag) {
        if (sol >= sag) return;
        double pivot = dizi[sag];
        int i = sol - 1;
        for (int j = sol; j < sag; j++) {
            if (dizi[j] > pivot) {
                i++;
                swap(dizi[i], dizi[j]);
                swap(idx[i], idx[j]);
            }
        }
        i++;
        swap(dizi[i], dizi[sag]);
        swap(idx[i], idx[sag]);
        quickSort(dizi, idx, sol, i-1);
        quickSort(dizi, idx, i+1, sag);
    }

    void sirala(double dizi[], int idx[]) {
        for (int i = 0; i < ulkeSayisi; i++) idx[i] = i;
        quickSort(dizi, idx, 0, ulkeSayisi - 1);
    }

public:
    EnerjiSistemi() : ulkeSayisi(0), ulkeler(nullptr), mesafe(nullptr) {} // constructor
    ~EnerjiSistemi() {                                                    //destructor
        if (ulkeler) delete[] ulkeler;
        if (mesafe) {
            for (int i = 0; i < ulkeSayisi; i++) delete[] mesafe[i];
            delete[] mesafe;
        }
    }
   // dosya okuma fonsksiyonları
    void verileriOku() {
        ifstream temp("ulkeler.txt");
        if (!temp.is_open()) {
            cout << "HATA: 'ulkeler.txt' dosyasi bulunamadi veya acilamadi!\n";
            return;
        }
        char dummy[512];
        temp.getline(dummy, 512);
        ulkeSayisi = 0;
        while (temp.getline(dummy, 512)) ulkeSayisi++;
        temp.close();

        ulkeler = new Ulke[ulkeSayisi];
        mesafe = new double*[ulkeSayisi];
        for (int i = 0; i < ulkeSayisi; i++) {
            mesafe[i] = new double[ulkeSayisi];
            for (int j = 0; j < ulkeSayisi; j++) mesafe[i][j] = -1;
        }

        ifstream f("ulkeler.txt");
        f.getline(dummy, 512);
        for (int i = 0; i < ulkeSayisi; i++) {
            f.getline(dummy, 512);
            char* t = strtok(dummy, " \t");
            int k = 0;
            while (t) {
                if (k == 0) ulkeler[i].id = atoi(t);
                else if (k == 1) strcpy(ulkeler[i].ad, t);
                else if (k == 2) ulkeler[i].uretim = atof(t);
                else if (k == 3) ulkeler[i].tuketim = atof(t);
                else if (k == 4) ulkeler[i].cezaKatsayisi = atof(t);
                else if (k == 5) ulkeler[i].satisFiyati = atof(t);
                t = strtok(NULL, " \t");
                k++;
            }
            ulkeler[i].cezaOdediMi = false;
            ulkeler[i].alinanMiktar = 0;
            ulkeler[i].kaynaktan = -1;
        }
        f.close();

        ifstream h("enerjihatlari.txt");
        if (!h.is_open()) {
            cout << "HATA: 'enerjihatlari.txt' dosyasi bulunamadi veya acilamadi!\n";
            return;
        }
        for (int i = 0; i < ulkeSayisi; i++) {
            char satir[4096];
            h.getline(satir, 4096);
            char* t = strtok(satir, ",\t ");
            int j = 0;
            while (t && j < ulkeSayisi) {
                if (strcmp(t, "-") == 0 || strcmp(t, "") == 0) mesafe[i][j] = -1;
                else mesafe[i][j] = atof(t);
                t = strtok(NULL, ",\t ");
                j++;
            }
        }
        h.close();
        cout << "Toplam " << ulkeSayisi << " ulke yuklendi.\n";
    }
             // burda en ideal seçim yapılır 
    void dengelemeYap() {
        for (int i = 0; i < ulkeSayisi; i++)
            ulkeler[i].netEnerji = ulkeler[i].uretim - ulkeler[i].tuketim;

        for (int i = 0; i < ulkeSayisi; i++) {
            if (ulkeler[i].netEnerji >= 0) continue;
            double eksik = -ulkeler[i].netEnerji;
            double ceza = eksik * ulkeler[i].cezaKatsayisi;

            double enUcuz = ceza + 1;
            int kaynak = -1;
            double miktar = 0;

            for (int j = 0; j < ulkeSayisi; j++) {
                if (i == j || ulkeler[j].netEnerji <= 0 || mesafe[i][j] < 0) continue;
                double al = min(ulkeler[j].netEnerji, eksik);
                double maliyet = al * ulkeler[j].satisFiyati * (mesafe[i][j] / 100.0);
                if (maliyet < enUcuz) {
                    enUcuz = maliyet;
                    kaynak = j;
                    miktar = al;
                }
            }

            if (kaynak != -1) {
                ulkeler[i].maliyet = enUcuz;
                ulkeler[i].alinanMiktar = miktar;
                ulkeler[i].kaynaktan = kaynak;
                ulkeler[i].cezaOdediMi = false;
                ulkeler[kaynak].netEnerji -= miktar;
                ulkeler[i].netEnerji += miktar;
            } else {
                ulkeler[i].maliyet = ceza;
                ulkeler[i].cezaOdediMi = true;
            }
        }

        for (int i = 0; i < ulkeSayisi; i++) {
            double satilan = ulkeler[i].uretim - ulkeler[i].tuketim - ulkeler[i].netEnerji;
            ulkeler[i].karZarar = (satilan > 0) ? satilan * ulkeler[i].satisFiyati : -ulkeler[i].maliyet;
        }

        double toplamCeza = 0;
        for (int i = 0; i < ulkeSayisi; i++)
            if (ulkeler[i].cezaOdediMi) toplamCeza += ulkeler[i].maliyet;

        cout << "\nDengeleme tamamlandi!\n";
        if (toplamCeza == 0)
            cout << "TEBRIKLER! Tum ulkeler enerji acigini karsiladi. CEZA ODEYEN YOK!\n";
        else
            cout << "Dengeleme tamamlandi. Toplam ceza: " << toplamCeza << " Euro\n";
    }
                // ülkelerin eerji üretimini hesaplar
    void uretimRaporu() {
        double* d = new double[ulkeSayisi];
        int* idx = new int[ulkeSayisi];
        for (int i = 0; i < ulkeSayisi; i++) { d[i] = ulkeler[i].uretim; idx[i] = i; }
        quickSort(d, idx, 0, ulkeSayisi-1);

        cout << "\n=== URETIM RAPORU (En yuksekten en dusuge) ===\n";
        ofstream out("uretim_raporu.txt");
        out << "EN YUKSEK URETIMDEN EN DUSUGE\n\n";
        for (int i = 0; i < ulkeSayisi; i++) {
            cout << i+1 << ". " << ulkeler[idx[i]].ad << " - " << d[i] << " GWh\n";
            out << i+1 << ". " << ulkeler[idx[i]].ad << " - " << d[i] << " GWh\n";
        }
        out.close();
        delete[] d; delete[] idx;
    }
  // ülkelerin enerji tüketimini hesaplar
    void tuketimRaporu() {
        double* d = new double[ulkeSayisi];
        int* idx = new int[ulkeSayisi];
        for (int i = 0; i < ulkeSayisi; i++) { d[i] = ulkeler[i].tuketim; idx[i] = i; }
        quickSort(d, idx, 0, ulkeSayisi-1);

        cout << "\n=== TUKETIM RAPORU (En yuksekten en dusuge) ===\n";
        ofstream out("tuketim_raporu.txt");
        out << "EN YUKSEK TUKETIMDEN EN DUSUGE\n\n";
        for (int i = 0; i < ulkeSayisi; i++) {
            cout << i+1 << ". " << ulkeler[idx[i]].ad << " - " << d[i] << " GWh\n";
            out << i+1 << ". " << ulkeler[idx[i]].ad << " - " << d[i] << " GWh\n";
        }
        out.close();
        delete[] d; delete[] idx;
    }
        // satın almaları rapolar
    void satinAlmaRaporu() {
        cout << "\n=== SATIN ALMA RAPORU ===\n";
        cout << "Alici                Miktar(GWh)     Maliyet(Euro)     Kaynak\n";
        cout << "----------------------------------------------------------------\n";
        ofstream out("satin_alma_raporu.txt");
        out << "ENERJI SATIN ALAN ULKELER\n\n";
        out << "Alici                Miktar(GWh)     Maliyet(Euro)     Kaynak\n";
        out << "----------------------------------------------------------------\n";

        for (int i = 0; i < ulkeSayisi; i++) {
            if (ulkeler[i].alinanMiktar > 0) {
                cout << left << setw(20) << ulkeler[i].ad 
                     << setw(16) << ulkeler[i].alinanMiktar 
                     << setw(18) << ulkeler[i].maliyet 
                     << ulkeler[ulkeler[i].kaynaktan].ad << "\n";
                out << left << setw(20) << ulkeler[i].ad 
                    << setw(16) << ulkeler[i].alinanMiktar 
                    << setw(18) << ulkeler[i].maliyet 
                    << ulkeler[ulkeler[i].kaynaktan].ad << "\n";
            }
        }
        out.close();
    }
  // ceza ödeyen ülkeleri rapolar
    void cezaRaporu() {
        cout << "\n=== CEZA RAPORU ===\n";
        ofstream out("ceza_raporu.txt");
        out << "CEZA ODEYEN ULKELER\n\n";

        double toplamCeza = 0;
        for (int i = 0; i < ulkeSayisi; i++) {
            if (ulkeler[i].cezaOdediMi) {
                toplamCeza += ulkeler[i].maliyet;
            }
        }

        if (toplamCeza == 0) {
            cout << "CEZA ODEYEN ULKE YOKTUR! (Optimal dengeleme saglandi)\n";
            out << "CEZA ODEYEN ULKE YOKTUR! (Optimal dengeleme saglandi)\n";
        } else {
            cout << "Ulke                 Eksik(GWh)      Ceza(Euro)\n";
            cout << "---------------------------------------------------\n";
            out << "Ulke                 Eksik(GWh)      Ceza(Euro)\n";
            out << "---------------------------------------------------\n";
            for (int i = 0; i < ulkeSayisi; i++) {
                if (ulkeler[i].cezaOdediMi) {
                    double eksik = ulkeler[i].tuketim - ulkeler[i].uretim;
                    cout << left << setw(20) << ulkeler[i].ad 
                         << setw(16) << eksik 
                         << ulkeler[i].maliyet << "\n";
                    out << left << setw(20) << ulkeler[i].ad 
                        << setw(16) << eksik 
                        << ulkeler[i].maliyet << "\n";
                    }
            }
            cout << "\nTOPLAM CEZA: " << toplamCeza << " Euro\n";
            out << "\nTOPLAM CEZA: " << toplamCeza << " Euro\n";
        }
        out.close();
    }
    
  // genel olarak istenleri rapolar
 void genelRapor() {
    double toplamSatinAlma = 0, toplamCeza = 0;
    for (int i = 0; i < ulkeSayisi; i++) {
        if (ulkeler[i].cezaOdediMi) toplamCeza += ulkeler[i].maliyet;
        else if (ulkeler[i].alinanMiktar > 0) toplamSatinAlma += ulkeler[i].maliyet;
    }

    // Kar/zarar dizisi oluştur ve sırala (büyükten küçüğe)
    double* deger = new double[ulkeSayisi];
    int* sira = new int[ulkeSayisi];
    for (int i = 0; i < ulkeSayisi; i++) {
        deger[i] = ulkeler[i].karZarar;
        sira[i] = i;
    }
    quickSort(deger, sira, 0, ulkeSayisi-1);

    // maliyet.txt
    ofstream maliyet("maliyet.txt");
    maliyet << fixed << setprecision(0);
    maliyet << "Toplam Satin Alma Maliyeti: " << toplamSatinAlma << " Euro\n";
    maliyet << "Toplam Ceza Maliyeti     : " << toplamCeza << " Euro\n";
    maliyet.close();

    // sonuc.txt 
    ofstream sonuc("sonuc.txt");
    sonuc << fixed << setprecision(0);

    cout << fixed << setprecision(0);
    cout << "\n=== GENEL RAPOR ===\n";
    cout << "Toplam Satin Alma Maliyeti: " << toplamSatinAlma << " Euro\n";
    cout << "Toplam Ceza Maliyeti     : " << toplamCeza << " Euro\n";
    if (toplamCeza == 0) cout << "TEBRIKLER! Optimal dengeleme saglandi!\n";

    cout << "\nKAR EDEN ULKELER:\n";
    sonuc << "KAR EDEN ULKELER:\n";
    int karSay = 0;
    for (int i = 0; i < ulkeSayisi && deger[i] > 0; i++) {
        karSay++;
        cout << karSay << ". " << ulkeler[sira[i]].ad << " -> +" << deger[i] << " Euro\n";
        sonuc << karSay << ". " << ulkeler[sira[i]].ad << " -> +" << deger[i] << " Euro\n";
    }
    if (karSay == 0) {
        cout << "   (Kar eden ulke bulunamadi)\n";
        sonuc << "   (Kar eden ulke bulunamadi)\n";
    }

    cout << "\nZARAR EDEN ULKELER:\n";
    sonuc << "\nZARAR EDEN ULKELER:\n";
    int zararSay = 0;
    for (int i = ulkeSayisi-1; i >= 0 && deger[i] < 0; i--) {
        zararSay++;
        cout << zararSay << ". " << ulkeler[sira[i]].ad << " -> " << deger[i] << " Euro\n";
        sonuc << zararSay << ". " << ulkeler[sira[i]].ad << " -> " << deger[i] << " Euro\n";
    }
    if (zararSay == 0) {
        cout << "   (Zarar eden ulke bulunamadi)\n";
        sonuc << "   (Zarar eden ulke bulunamadi)\n";
    }

    cout << "\n";
    sonuc.close();
    delete[] deger; delete[] sira;
}
// consolda çıkan seçenek menüsü
    void menu() {
        cout << "\n=== AVRUPA ENERJI SISTEMI ===\n";
        cout << "1. Verileri Oku\n";
        cout << "2. Dengeleme Yap\n";
        cout << "3. Uretim Raporu\n";
        cout << "4. Tuketim Raporu\n";
        cout << "5. Satin Alma Raporu\n";
        cout << "6. Ceza Raporu\n";
        cout << "7. Genel Rapor\n";
        cout << "8. Cikis\n";
        cout << "Seciminiz: ";
    }
};

EnerjiSistemi sistem;

int main() {
    int secim;
    bool veriOkundu = false, dengelemeYapildi = false;

    do {
        sistem.menu();
        cin >> secim;

        switch(secim) {
            case 1:
                sistem.verileriOku();
                veriOkundu = true;
                break;
            case 2:
                if (veriOkundu) {
                    sistem.dengelemeYap();
                    dengelemeYapildi = true;
                } else cout << "Once verileri oku!\n";
                break;
            case 3:
                if (dengelemeYapildi) sistem.uretimRaporu();
                else cout << "Once dengeleme yap!\n";
                break;
            case 4:
                if (dengelemeYapildi) sistem.tuketimRaporu();
                else cout << "Once dengeleme yap!\n";
                break;
            case 5:
                if (dengelemeYapildi) sistem.satinAlmaRaporu();
                else cout << "Once dengeleme yap!\n";
                break;
            case 6:
                if (dengelemeYapildi) sistem.cezaRaporu();
                else cout << "Once dengeleme yap!\n";
                break;
            case 7:
                if (dengelemeYapildi) sistem.genelRapor();
                else cout << "Once dengeleme yap!\n";
                break;
            case 8:
                cout << "Program sonlandi. Basarilar!\n";
                break;
            default:
                cout << "Gecersiz secim!\n";
                break;
        }
    } while (secim != 8);

    return 0;
}