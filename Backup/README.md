Для того чтобы не допустить потерю информации при порче диска, обычно используют резервное копирование файлов (backup). Простейшей формой backup’а является копирование всех файлов из одной директории в другую. Этот способ требует много времени и места на диске. Напишите программу, осуществляющую более интеллектуальный подход.

Программа должна брать из командной строки два параметра: исходную директорию и директорию назначения. Она должна рекурсивно сканировать исходную директорию, делать копии всех файлов, для которых ранее не делались копии или которые были изменены с момента последнего backup’а, размещая их в соответствующих местах директории назначения в сжатом виде (например в gzip). Это уменьшит требуемый размер дисковой памяти; а файл будет переименован с добавлением расширения .gz (или другим соответствующим алгоритму сжатия). Все возникающие ошибки (нет исходной директории, файл закрыт для чтения и т.д.) должны корректно обрабатываться с выдачей соответствующего сообщения. 