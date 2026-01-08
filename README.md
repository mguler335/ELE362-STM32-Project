# 🛠️ STM32 Embedded Systems Project: Peripheral Control & AI Integration
![IMG_3965](https://github.com/user-attachments/assets/d2a52c39-e6a7-4d39-81fe-39b228a9c3c9)


Bu depo, STM32 NUCLEO-F091RC geliştirme kartı kullanılarak gerçekleştirilmiş üç bağımsız gömülü sistem projesini içermektedir. Proje; UART haberleşmesi, analog-dijital dönüşümler (ADC/DAC), kesme tabanlı asansör kontrolü ve yapay zeka ile Mors kodu çözme konularını kapsamaktadır.

## 🚀 Proje İçeriği

### 📡 Proje 1: UART, DAC ve ADC Senkronizasyonu
Mikrodenetleyici üzerindeki farklı haberleşme ve analog birimlerin zamanlayıcılar (Timers) ile senkronize çalışmasını sağlar.
* [cite_start]**UART Haberleşme:** USART3 üzerinden gönderilen 8-bitlik veriler USART1 üzerinden kesme (interrupt) modunda alınır[cite: 311, 312].
* [cite_start]**DAC Kontrol:** Gelen verinin büyüklüğüne göre (0-255) DAC çıkışı 1V, 2V veya 3V seviyelerine ayarlanır[cite: 313].
* [cite_start]**ADC Örnekleme:** DAC çıkışı, TIM3 zamanlayıcısının donanımsal tetiklemesiyle (Hardware Trigger) her 0.5 saniyede bir 8-bit çözünürlükte okunur[cite: 315].

### 🛗 Proje 2: Akıllı Asansör Simülasyonu
Keypad ve LED animasyonları kullanılarak 0-9 katları arasında çalışan bir asansör sistemidir.
* [cite_start]**Ara Durak Mantığı:** Asansör bir yöne hareket ederken güzergah üzerindeki yeni talepleri önceliklendirir[cite: 270, 329].
* [cite_start]**Zamanlama:** İşlemciyi kilitlemeyen (non-blocking) kesme tabanlı bir mimari kullanılmıştır[cite: 321].
* [cite_start]**LED Durumları:** Yükselme (0.5 sn toggle), alçalma (0.2 sn toggle) ve durma (sabit yanma) durumları LED2 üzerinden izlenebilir[cite: 262, 263].

### 🧠 Proje 3: AI Tabanlı Mors Kodu Çözücü
Gömülü bir derin öğrenme modeli (X-CUBE-AI) kullanarak buton sinyallerini gerçek zamanlı olarak metne dönüştürür.
* [cite_start]**Derin Öğrenme:** Google Colab üzerinde eğitilen model, X-CUBE-AI eklentisi ile STM32'ye entegre edilmiştir[cite: 275, 276].
* [cite_start]**Sınıflandırma:** Nokta, Çizgi, Çift Tıklama ve Boşluk olmak üzere 4 farklı giriş sınıflandırılır[cite: 283].
* [cite_start]**Veri İşleme:** Her 25 ms'de bir örnekleme yapılarak 1 saniyelik pencereler halinde model girişi oluşturulur[cite: 284, 285].

## 🛠️ Donanım ve Yazılım Yapısı
* [cite_start]**Kart:** STM32 NUCLEO-F091RC[cite: 274].
* **IDE:** STM32CubeIDE.
* [cite_start]**Eklenti:** X-CUBE-AI (Makine Öğrenmesi için)[cite: 288].
* **Haberleşme:** UART (115200 Baud).

## 🔌 Pin Bağlantıları
| Bileşen | Pin | Açıklama |
| :--- | :--- | :--- |
| **UART Verici (Tx)** | USART3 | [cite_start]Veri gönderimi [cite: 311] |
| **UART Alıcı (Rx)** | USART1 | [cite_start]Veri alımı [cite: 312] |
| **Buton** | PC13 | [cite_start]Mors kodu girişi (Mavi buton) [cite: 292] |
| **LED** | PA5 | [cite_start]Asansör hareket durumu (LD2) [cite: 260] |
| **DAC Çıkışı** | PA4 | Analog voltaj üretimi |

## 📦 Kurulum ve Çalıştırma
1. STM32CubeIDE projesini içe aktarın.
2. `X-CUBE-AI` paketinin yüklü olduğundan emin olun.
3. [cite_start]Proje 3 için eğitilen `.tflite` modelini eklenti üzerinden yükleyin[cite: 289].
4. Kodu derleyin ve kartınıza yükleyin.
5. [cite_start]`Debugger` ekranı üzerinden verileri ve asansörün anlık kat bilgisini izleyebilirsiniz[cite: 252, 267].

---
[cite_start]*Bu çalışma Muhammed Halil Güler tarafından TOBB ETÜ ELE 362 dersi kapsamında geliştirilmiştir.* [cite: 367]
