# Оптимизация работы Альфа-смешения

***Целью данной работы является ускорение вычислений, проводимых во время альфа-смешения, за счет параллелизации процессов работы с пикселями.***


![](/images&font/alpha_blending_result.png?raw=true "Пример альфа-смешения изображений теннисного стола и кота")

## **Что такое альфа-смешение**

Альфа-смешение - это метод наложения одного изображения на другое, использующее альфа-канал изображения на переднем плане как маску. Альфа-канал определяет степень прозрачности пикселя.

Ниже представлена формула, по которой производились вычисления цвета:
$$mixed.color = foreground.color \cdot foreground.alpha + background.color \cdot (1 - foreground.alpha),$$
где foreground_alpha - значение в интервале от 0 до 1, прямо пропорционально зависящее от значения альфа-канала.


## **Стадии работы**

Для анализа времени работы программы в том или ином случае было написано несколько версий программы, осуществляющей альфа-смешение.

Каждая версия придерживается следующей структуры:
1. Получение начальных изображений.
2. Создание заготовки под изображение - результат выполнения программы, представляющей собой копию изображения на заднем плане.
3. Конвертирование пикселей в нужной части изображения по формуле выше.
4. Конвертирование полученного изображения в нужный формат, если необходимо.

### **Наивная версия**

Наивная версия ([naive version link](/implementation_versions/alpha_blending_0.cpp)) работает с SFML структурой Image и обращается к ней каждый раз, когда необходимо получить или загрузить пиксель. Это значительно замедляет время работы во время вычислений, но простота кода программы позволяет увидеть алгоритм реализации альфа-смешения наиболее ярко.

### **Обычная версия**

Эта версия ([ordinary version link](/implementation_versions/alpha_blending_1.cpp)) работает с массивами пикселей, что позволяет обращаться к ним гораздо быстрее, чем в случае наивной версии. Также следует отметить, что теперь необходимо конвертировать изображения из BGRA формата (формат .bmp файлов) в RGBA формат (формат, требуемый для создания изображения в SFML), так как на вход в нашем случае подаются .bmp файлы.

### **Оптимизированная версия**

Оптимизированная версия ([optimized version link](/implementation_versions/alpha_blending_2.cpp)) представляет собой предыдущую версию, в которой вычисления с пикселями проводятся с использованием интринсик-функции, что позволяет вычислять несколько пикселей итогового изображения одновременно. Мы используем SSE инструкции для работы с 128-битными XMM регистрами, что позволяет вычислять 4 пикселя за одну итерацию.

***Замечание: в программе альфа-смешения были оптимизированы только вычисления, преобразующие цвет итогового пикселя, и мы замеряли только их время работы***

## **Время работы**

Некоторые важные замечания:
- Рабочий fps измерялся без учета отрисовки и вычислялся как обратная величина ко времени работы (в секундах):

$$fps = \frac {1}{work.time} ± RMS,$$

где под временем работы (work.time) программы подразумевается время, потраченное на вычисления, с которыми проводилась оптимизация, 
а под RMS (root mean square) - среднеквадратическое отклонение среднего значения fps, основывающееся на результатах выполнения программы

- Среднее время измерялось посредством многократного запуска программы и усреднения результатов (10000 запусков в случае наивной версии и 100000 в других).
- Все версии запускались с флагом оптимизации -O3.

| Версия                      | FPS        | Коэффициент ускорения |
| --------------------------- |:----------:|:---------------------:|
| Наивная                     | 423  ± 53  |    0.30               |
| Не оптимизированная         | 1411 ± 95  |    1                  |    
| Оптимизированная            | 4212 ± 391 |    2.99               |

## **Обсуждения и выводы**

### **Сравнение ассемблерного кода программ**

Рассмотрим основной цикл отрисовки в случаях обычной и оптимизированной версии и его ассемблерный код с помощью программы godbolt.org (рассуждения и код предоставлены для оптимизации с флагом -O2).

*Сверху представлена обычная версия, снизу - оптимизированная.*

| Код на Си                                                 |  Соответствующий ассемблерный код                            |
|:---------------------------------------------------------:|:------------------------------------------------------------:|
| ![](/images&font/godbolt_cmp/not_optimized_C.png?raw=true)| ![](/images&font/godbolt_cmp/not_optimized_asm.png?raw=true) |
| ![](/images&font/godbolt_cmp/optimized_C.png?raw=true)    | ![](/images&font/godbolt_cmp/optimized_asm.png?raw=true)     |


Проанализировав и сравнив ассемблерный код можно сделать следующие выводы:
- Действительно, оптимизация преобразует обработку одного пикселя за итерацию в обработку четырех за счет применения интринсик-функций.
- В случае оптимизированной интринсик-функциями версии производится гораздо меньше пересылок с памятью (10 - в случае обычной версии, 3 - в случае оптимизированной) за счет хранения значений в XMM регистрах.

***Замечание: компилятор использует интринсик-функции в обычной версии при компиляции с флагом -O3, но ассемблерный код в этом случае довольно длинный и сложный для понимания, поэтому здесь мы его рассматривать не будем.***



### **Общие выводы**
- Использование интринсик-функций ускоряет работу программы в тех местах, где необходимо проводить одинаковые действия с несколькими независимыми объектами, а также способно уменьшить количество обращений к памяти, которые осуществляются в течении длительного (относительно других инструкций) времени, при исполнении.
- На практике, накладные расходы не позволяют достигнуть абсолютного ускорения работы программы: обрабатывая в 4 раза больше пикселей за одну итерацию с помощью XMM регистров мы получили ускорение только в 3 раза (в сравнении с обычной версией). 