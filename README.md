# 🛠️ STM32 Embedded Systems Project: Peripheral Control & AI Integration
![IMG_3965](https://github.com/user-attachments/assets/d2a52c39-e6a7-4d39-81fe-39b228a9c3c9)


# 🛠️ STM32 Embedded Systems: Peripheral Control, Elevator Logic & AI

Bu proje, **TOBB ETÜ ELE 362 / BİL 326 Mikroişlemciler** dersi kapsamında STM32 NUCLEO-F091RC geliştirme kartı üzerinde gerçekleştirilmiş, birbirinden bağımsız üç farklı gömülü sistem projesini içermektedir.

## 🚀 Proje İçerikleri

### 📡 Proje 1: UART, DAC ve ADC Senkronizasyonu
Mikrodenetleyici üzerindeki farklı haberleşme ve analog birimlerin zamanlayıcılar (Timers) ile uyumlu çalışmasını sağlar.
* **Dizi Tanımlama:** 0-255 arası değerlerden oluşan 50 elemanlı bir veri seti kullanılır.
* **UART Loopback:** USART3 (Verici) ve USART1 (Alıcı) birimleri birbirine bağlanarak veri iletimi sağlanır.
* **Voltaj Kontrolü (DAC):** Alınan verinin değerine göre (0-100, 101-200, >200) DAC çıkışından sırasıyla 1V, 2V veya 3V gerilim üretilir.
* **Hassas Okuma (ADC):** DAC çıkışı ADC girişine bağlanır ve TIM3 tetiklemesiyle her 0.5 saniyede bir okuma yapılır.

### 🛗 Proje 2: Akıllı Asansör Simülasyonu
Keypad ve LED animasyonları ile bir asansörün çalışma prensibini simüle eder.
* **Hareket Mantığı:** 0-9 katları arasında hareket eden asansör, yükselirken 0.5 sn, alçalırken 0.2 sn aralıklarla LED yanıp sönerek (Toggle) durumunu belli eder.
* **Akıllı Durak Sistemi:** Asansör hareket halindeyken güzergah üzerindeki yeni talepleri algılar. Örneğin 0'dan 7'ye çıkarken 5. kattan talep gelirse, önce 5. katta durur ve sonra hedefine devam eder.
* **Zamanlama:** HAL_Delay yerine tamamen Timer Interrupt yapısı kullanılarak sistemin aynı anda hem buton okuması hem de hareket etmesi sağlanmıştır.

### 🧠 Proje 3: AI Tabanlı Mors Kodu Çözücü
Gömülü bir derin öğrenme modeli (X-CUBE-AI) kullanarak buton sinyallerini metne dönüştürür.
* **Derin Öğrenme Modeli:** Google Colab üzerinde eğitilen ve 4 farklı girişi (Nokta, Çizgi, Çift Tık, Boş) tanıyan model, STM32'ye entegre edilmiştir.
* **Gerçek Zamanlı İşleme:** PC13 mavi butonu üzerinden her 25 ms'de bir örnek alınır ve 1 saniyelik veriler model tarafından analiz edilir.
* **Cümle Çözümleme:** Boşluk sinyali harf sonunu, çift tıklama sinyali ise kelime sonunu ifade eder ve sonuç string olarak saklanır.

## 🛠️ Teknik Detaylar
* **Geliştirme Kartı:** STM32 NUCLEO-F091RC
* **Yazılım Geliştirme:** STM32CubeIDE & X-CUBE-AI
* **Haberleşme:** UART (115200 Baudrate)
* **Zamanlayıcılar:** TIM3 (ADC ve Veri Toplama), TIM6 (UART Gönderimi)

## 🔌 Donanım Kurulumu
1. **UART Bağlantısı:** USART3 Tx pinini USART1 Rx pinine fiziksel olarak bağlayın.
2. **Analog Bağlantı:** PA4 (DAC Out) pinini PA0 (ADC In) pinine bir jumper kablo ile bağlayın.
3. **Keypad:** Proje 2 için asansör hedef kat girişlerini yapacak keypad bağlantısını tamamlayın.
4. **Debug:** Sonuçları ve kat bilgilerini izlemek için STM32CubeIDE üzerinden Debugger/Live Expressions ekranını kullanın.

---
