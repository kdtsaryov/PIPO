Сортировка слиянием

Исходные данные: массив объектов, функция для их сравнения, количество вычислителей.

Цель: написать программу для упорядочивания массива параллельно несколькими вычислителями. 

Предполагаемый алгоритм распараллеливания: при сортировке слиянием массив делится на две равных по размеру части, которые сортируются отдельно и сливаются в один. Таким образом, получается дерево рекурсии.