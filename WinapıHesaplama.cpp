#include <iostream>
#include <vector>
#include <random>
#include <windows.h>
#include <chrono>

struct KonvolüsyonArgs {
    const std::vector<double>* x;
    const std::vector<double>* h;
    std::vector<double>* y;
    size_t baslangic_n;
    size_t bitis_n;// threadler arasında paylaşılan parametreler
};

DWORD WINAPI konvolüsyon_thread(LPVOID args) {
    KonvolüsyonArgs* konv_args = static_cast<KonvolüsyonArgs*>(args);
    const std::vector<double>& x = *konv_args->x;
    const std::vector<double>& h = *konv_args->h;
    std::vector<double>& y = *konv_args->y;
    for (size_t n = konv_args->baslangic_n; n < konv_args->bitis_n; ++n) { // bir thread'in konvolüsyonu belirli bir aralıkta gerçekleştirmesini sağlar
        for (size_t k = 0; k < h.size(); ++k) {
            if (n >= k && n - k < x.size()) {
                y[n] += x[n - k] * h[k];
            }
        }
    }
    return 0;
}

int main() {
    setlocale(LC_ALL, "Turkish");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-5, 5); //x ve h değerini rastgele atma

    std::vector<double> x(10);//N değeri
    std::vector<double> h(10);//M değeri
    for (auto& xi : x) xi = dis(gen);
    for (auto& hi : h) hi = dis(gen);

    std::vector<double> y(x.size() + h.size() - 1, 0);

    size_t threads_sayisi = 16;// Thread sayısı
    std::vector<HANDLE> threads(threads_sayisi);
    std::vector<KonvolüsyonArgs> thread_args(threads_sayisi);
    size_t n_bölü_thread = y.size() / threads_sayisi;

    auto baslangic_hesaplama = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < threads_sayisi; ++i) {
        thread_args[i] = { &x, &h, &y, i * n_bölü_thread, (i + 1) * n_bölü_thread };
        if (i == threads_sayisi - 1) thread_args[i].bitis_n = y.size();
        threads[i] = CreateThread(NULL, 0, konvolüsyon_thread, &thread_args[i], 0, NULL); // Paralel(yapılan işlemler thread sayısına bölünür, her bir thread kendi hesaplamasını yapıp y vektörüne gönderir)
    }

    for (auto& thread : threads) WaitForSingleObject(thread, INFINITE);//her bir thread'in tamamlanması beklemesi için

    auto bitis_hesaplama = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> hesaplama_fark = bitis_hesaplama - baslangic_hesaplama;//hesaplama süresi

    std::cout << "N: " << x.size() << ", M: " << h.size() << ", Thread Sayisi: " << threads_sayisi << ", Hesaplama Suresi: " << hesaplama_fark.count() << " saniye\n";
    std::cout << "Hesaplama İşlemi Tamamlandı. Kapatmak İçin Enter Tuşuna Basınız!" << "\n";

    std::cin.get();

    return 0;
}
