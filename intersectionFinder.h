#pragma once 
#include "library.h"

 
 
class IntersectionFinder
{
protected:

	enum Color{ white, gray, black };

	typedef struct Intersection {
		size_t wordIndex1;		//Индекс слова, которое пересекают
		size_t wordIndex2;		// Индекс слова, с которым есть пересечение
		size_t charIndex1;		// Индекс буквы в текущем слове
		size_t charIndex2;		// Индекс буквы в пересекающемся слове 
		bool isEmpty = true;
		bool used = false;
		int iback = -1;         //нет родителя = -1
		Color color = Color::white;	
	} 
	IntersectionInfo;
	 

	size_t numWords;
	vector<string> words = { "cross", "word", "test", "grid", "code", "data", "logic" };
	vector<unordered_map<char, vector<size_t>>> wordCharMap ; 
	std::vector<std::vector<vector<IntersectionInfo>>> intersectionMatrix;  //Матрица пересечений
	vector<size_t> wordsIntersectionRating;
	vector<size_t> indeces ;
	unordered_map<size_t, vector<IntersectionInfo>> graph;  
	vector<vector<char>> grid;


	const int GRID_SIZE = 20;
	const int TBound = 1000; // Временное ограничение (в миллисекундах)
	const int PBound = 50;   // Прогресс-ограничение
	const int HBound = 2;    // Минимальное количество пересекающихся слов


	//Быстрый доступ к списку слов, содержащих определённую букву.
	void createHashTable() {
		//typedef struct Intersection {
		//  size_t wordIndex1;  // Индекс слова,  которое  пересекли
		//	size_t wordIndex2;  // Индекс слова, с которым есть пересечение
		//	size_t charIndex1; // Индекс буквы в текущем слове
		//	size_t charIndex2; // Индекс буквы в пересекающемся слове
		//} IntersectionInfo;
		//std::vector<std::vector<vector<IntersectionInfo>>> intersectionMatrix;

		// предобработки данных для быстрого поиска общих букв между словами
		// Создание хеш-таблицы пересечения слов
		// vector<unordered_map<char, vector<size_t>>> wordCharMap(numWords);
		wordCharMap.resize(numWords);
		for (size_t i = 0; i < numWords; i++) {
			for (size_t j = 0; j < words[i].length(); j++) {
				wordCharMap[i][words[i][j]].push_back(j);
			}
		}
	}

	void createMatrix() {


		//std::vector<std::vector<vector<IntersectionInfo>>> intersectionMatrix; 

		intersectionMatrix.resize(numWords, std::vector<std::vector<IntersectionInfo>>(numWords));

		for (size_t i = 0; i < numWords; ++i) {
			for (size_t j = 0; j < numWords; ++j) {
				if (i != j) {
					// Ищем пересечения между словом i и словом j
					for (size_t index1 = 0; index1 < words[i].length(); ++index1) {
						char currentAlpha = words[i][index1];

						//cout << "Ищем пересечение: " << words[i] << " \n";
						bool isFounded = false;
						// Проверяем, есть ли эта буква в строке jой
						if (wordCharMap[j].count(currentAlpha)) {
							//vector<unordered_map<char, vector<size_t>>> wordCharMap(numWords);
							for (size_t index2 : wordCharMap[j][currentAlpha]) {
								// Нашли пересечение, добавляем слово в матрицу
								//cout << words[j] << " ";
								IntersectionInfo intersection = { i, j, index1, index2 };
								intersectionMatrix[i][j].push_back(intersection);
								isFounded = true;

							}
							//cout << endl;
						}
						//cout << endl;
					}
				}
			}
		}
	}
	 
	void createWordsIntersectionRating() {
		size_t size = words.size();
		wordsIntersectionRating.resize(size, 0);

		// Размещаем слова с числом пересечений по убыванию. используя жадную cтратегию

		// Подсчёт рейтинга только для исходящих пересечений
		for (size_t i = 0; i < intersectionMatrix.size(); i++) {
			for (size_t j = 0; j < intersectionMatrix[i].size(); j++) {
				if (!intersectionMatrix[i][j].empty()) {
					wordsIntersectionRating[i] += intersectionMatrix[i][j].size();
				}
			}
		}

		// Создание списка индексов для сортировки слов на основе рейтинга
		//vector<size_t> indeces(size);
		indeces.resize(numWords);
		for (size_t i = 0; i < size; i++) indeces[i] = i;



		// Сортировка индексов по убыванию рейтинга пересечений, затем по длине слова, затем по алфавиту
		sort(indeces.begin(), indeces.end(), [&](size_t a, size_t b) {
			if (wordsIntersectionRating[a] != wordsIntersectionRating[b])
				return wordsIntersectionRating[a] > wordsIntersectionRating[b];
			//Среди слов с одинаковым рейтингом пересечений приоритет отдается более длинным словам
			//Длинные слова размещаются в первую очередь, потому что для них проще найти подходящее место, 
			// пока сетка ещё относительно пустая.
			else if (words[a].size() != words[b].size())
				return words[a].size() > words[b].size();
			//Если слова имеют одинаковый рейтинг пересечений и одинаковую длину, они сортируются в алфавитном порядке
			else
				return words[a] < words[b]; // Алфавитный порядок
			});

		  
	}

	// Уменьшаем сложность задачи, делая её более управляемой!!!!!!
	// Оптимизация с помощью Жадного Алгоритма
	void createGraph() {


		//std::vector<std::vector<vector<IntersectionInfo>>> intersectionMatrix;

		//матрица смежности уже создана на основе пересечений между словами, 
		// и теперь мы можем использовать эту матрицу, чтобы построить граф.
		// Граф, где каждый узел (слово) связан с другими словами через пересечения
		//unordered_map<size_t, vector<IntersectionInfo>> graph;

		for (size_t newIndexI = 0; newIndexI < indeces.size(); ++newIndexI) {
			for (size_t newIndexJ = 0; newIndexJ < indeces.size(); ++newIndexJ) {
				size_t originalIndexI = indeces[newIndexI]; // Индекс в исходном порядке
				size_t originalIndexJ = indeces[newIndexJ]; // Индекс в исходном порядке

				if (!intersectionMatrix[originalIndexI][originalIndexJ].empty()) { // Если есть пересечения
					for (const auto& intersection : intersectionMatrix[originalIndexI][originalIndexJ]) {
						// Добавляем пересечение в граф, используя новый индекс (порядок)
						graph[newIndexI].push_back(intersection);
					}
				}
			}
		}
	}


	// Функция для размещения слова на сетке
	bool placeWord(vector<vector<char>>& grid, const string& word, int row, int col, bool horizontal) {

		// 1. Проверяем, что начальные координаты не выходят за пределы сетки
		if (row < 0 || row >= grid.size() || col < 0 || col >= grid[0].size()) {
			return false; // Слово выходит за границы
		}

		// 2. Проверяем возможность размещения слова
		if (horizontal) {
			for (size_t i = 0; i < word.size(); ++i) {
				if (col + i >= grid[0].size()) return false; // Выход за пределы сетки
				if (grid[row][col + i] != '.' && grid[row][col + i] != word[i]) return false;
			}
		}
		else {
			for (size_t i = 0; i < word.size(); ++i) {
				if (row + i >= grid.size()) return false; // Выход за пределы сетки
				if (grid[row + i][col] != '.' && grid[row + i][col] != word[i]) return false;
			}
		}

		// 3. Размещаем слово
		if (horizontal) {
			for (size_t i = 0; i < word.size(); ++i) {
				grid[row][col + i] = word[i];
			}
		}
		else {
			for (size_t i = 0; i < word.size(); ++i) {
				grid[row + i][col] = word[i];
			}
		}
		return true;
	}

	//Правило вычисления координат 
	pair<size_t, size_t> calculateCoordinates(const IntersectionInfo& intersection,
		size_t row_W1, size_t col_W1, bool horizontal_W1) {
		size_t row, col;

		if (horizontal_W1) {
			// W1 горизонтальное, W2 вертикальное
			row = row_W1 - intersection.charIndex2;//Это сдвиг вверх по сетке в W2 c которым пересек W1
			col = col_W1 + intersection.charIndex1;//Это сдвиг вправо от начальной позиции W1
		}
		else {
			// W1 вертикальное, W2 горизонтальное
			row = row_W1 + intersection.charIndex1;
			col = col_W1 - intersection.charIndex2;
		}

		return { row, col };
	}



	void printHashTable(vector<unordered_map<char, vector<size_t>>> wordCharMap) {
		cout << "хеш-таблица для каждого слова, ключ - буква, значение - индекс буквы в слове\n";
		for (size_t i = 0; i < wordCharMap.size(); i++) {
			for (auto pair : wordCharMap[i]) {
				auto key = pair.first;
				cout << "'" << key << "' ";
				for (auto index : pair.second) {
					cout << index << " " << words[i] << " ";
				}
				cout << endl;
			}
		}
	}
	//вывод матрицы пересечений
	void printMatrix() const {

		cout << "\nMatrix\n";
		const int cellWidth = 20; // Ширина для каждой ячейки

		// Заголовок для слов
		std::cout << std::setw(1.5 * cellWidth) << " ";
		for (size_t i = 0; i < words.size(); i++) {
			std::cout << std::setw(cellWidth) << words[i];
		}
		std::cout << std::endl;

		//std::vector<std::vector<vector<IntersectionInfo>>> intersectionMatrix;
		// 
		// Печать содержимого матрицы пересечений
		for (size_t i = 0; i < words.size(); i++) {

			std::cout << std::setw(cellWidth) << words[i];

			for (size_t j = 0; j < words.size(); j++) {
				if (i != j) {
					// Проверяем, является ли элемент пустым (если у вас есть критерий для "пустоты")
					if (!intersectionMatrix[i][j].empty()) { // Пример критерия "пустоты"

						std::cout << std::setw(cellWidth);

						for (const auto& elem : intersectionMatrix[i][j]) {
							std::cout << elem.charIndex1 << " x " << elem.charIndex2 << " ";
						}
					}
					else {
						// Печатаем пустую ячейку, если пересечений нет
						std::cout << std::setw(cellWidth) << "-";
					}
				}
				else {
					// Помечаем диагональные элементы
					std::cout << std::setw(cellWidth) << "xxx";
				}
			}
			std::cout << std::endl;
		}

		std::cout << std::endl;
	}

	// Вывод сетки
	void printGrid(const vector<vector<char>>& grid) {
		cout << "Cетка кроссворда:" << endl;
		for (const auto& row : grid) {
			for (const auto& cell : row) {
				cout << (cell == '.' ? '.' : cell) << " ";
			}
			cout << endl;
		}
		cout << endl;
	}

	void printGraph(unordered_map<size_t, vector<IntersectionInfo>> graph) {

		cout << "\nGraph\n";
		for (const auto& pair : graph) {
			cout << "Вершина: " << words[indeces[pair.first]] << " связана с:\n";
			for (const auto& intersection : pair.second) {
				cout << "  Слово: " << words[intersection.wordIndex2]
					<< " (пересечение на индексах " << intersection.charIndex1
					<< " и " << intersection.charIndex2 << ")\n";
			}
			cout << endl;
		}
		cout << endl;
	}
	 
	void printWordsIntersectionRating() {
		// Вывод отсортированных слов по убыванию рейтинга пересечений
		cout << "\nВывод рейтинга\n";

		for (size_t i = 0; i < wordsIntersectionRating.size(); i++) {
			cout << "Слово \"" << words[i] << "\" имеет рейтинг: " << wordsIntersectionRating[i] << endl;
		}
		cout << endl;
	}


public: 

	IntersectionFinder(vector<string>& words) {
		
		this->words = words;
		numWords = words.size();

		createHashTable();
		createMatrix(); 
		printMatrix();
		createWordsIntersectionRating();
		createGraph();
		printWordsIntersectionRating();
		printGraph(graph);
		 
		 
		createCrisCros();
		printGrid(grid);
	};
	~IntersectionFinder() { };
 
  
 
	

	void createCrisCros() {  
	 
	 
		// Очередь для обработки слов
		queue<size_t> wordQueue; 
		// Список размещённых слов
		unordered_set<size_t> placedWords;
		  
		//vector of vectors
		grid.resize(GRID_SIZE, vector<char>(GRID_SIZE, '.'));

		// Размещаем первое слово
		size_t firstWordIndex = indeces[0]; // Предполагаем, что первое слово — самое важное
		size_t row_W1 = GRID_SIZE / 2;
		size_t col_W1 = GRID_SIZE / 2 - words[firstWordIndex].length() / 2;
		bool horizontal_W1 = true;

		if (placeWord(grid, words[firstWordIndex], row_W1, col_W1, horizontal_W1)) {
			cout << "Слово \"" << words[firstWordIndex] << "\" размещено первым.\n";
			placedWords.insert(firstWordIndex); 
			wordQueue.push(firstWordIndex); // Добавляем в вектор очередь
		}
		else {
			cout << "Не удалось разместить первое слово.\n";
			return;
		}

		 

		// Обрабатываем пересечения через очередь
		while (!wordQueue.empty()) {
			 
			size_t currentWordIndex = wordQueue.front(); // Берём последнее слово из очереди
			wordQueue.pop(); 
			 
			 
			// Перебираем все пересечения для текущего слова
			for (const auto& intersection : graph[currentWordIndex]) {
				size_t nextWordIndex = intersection.wordIndex2;
 


				// Если слово уже размещено, пропускаем
				if (placedWords.find(nextWordIndex) != placedWords.end())  continue;
				 

				// Вычисляем координаты для размещения слова
				auto coords = calculateCoordinates(intersection, row_W1, col_W1, horizontal_W1);
				size_t nextRow = coords.first;
				size_t nextCol = coords.second;
				bool nextHorizontal = !horizontal_W1;

				// Пытаемся разместить слово
				if (placeWord(grid, words[nextWordIndex], nextRow, nextCol, nextHorizontal)) {

					cout << "Слово \"" << words[nextWordIndex] << "\" размещено через пересечение: ";
					cout << "(" << intersection.charIndex1 << " " << intersection.charIndex2 << ")\n";

					placedWords.insert(nextWordIndex); 

					//break; // Прекращаем поиск пересечений для текущего слова
				} 
				else
				{
					cout << "Не удалось разместить слово \"" << words[nextWordIndex] << "\".\n";  
					//wordQueue.push(nextWordIndex); // Добавляем слово в очередь
					// Рекурсия???
					//Удалить последнее слово из очереди, пробуем разместить с др пересечением
					//Если список пересечений для слова исчерпан
					// 
					//Удялем последнее слово из очереди, пробуем разместить с др пересечением
				}
			} 

		}
   
		 
		return;
	}
	
 
	
};

/*
	*
		 https://litresp.ru/chitat/ru/%D0%A3/uezerell-charlz/etyudi-dlya-programmistov/9
		https://static.aminer.org/pdf/PDF/000/258/751/practical_crossword_generation_with_checkpoint_search.pdf


		инициализировать сетку
		инициализировать граф со словами и возможными пересечениями
		TBound = <временное ограничение>
		PBound = <ограничение по прогрессу>
		HBound = <порог для минимального числа пересекающихся слов>

		повторять
			оценить неполные шаблоны (позиции в сетке, где могут быть размещены слова)
    
			выбрать неполный шаблон P с наименьшим количеством вариантов (оценка S)
			если S > HBound:
				отметить текущий узел (P) как контрольную точку (то есть белый узел)
    
			если в P еще не размещено слово:
				W = выбрать случайное слово из списка слов, пересекающихся в P
			иначе, если для P есть еще подходящие слова и количество выбранных слов ≤ коэффициент ветвления:
				W = выбрать следующее подходящее слово для шаблона P
			иначе:
				S = 0  # исчерпаны подходящие слова; форсируем откат

			если S == 0:
				оценить прогресс
				если время > TBound и прогресс < PBound:
					откат к ближайшему отмеченному узлу
				иначе:
					откат к родительскому узлу (предыдущее слово)

			иначе:
				разместить слово W в шаблоне P на сетке
				обновить пересекающиеся шаблоны (корректировать позиции пересекающихся слов, если нужно)
				отметить шаблон P как заполненный (указывая, что слово размещено)
        
		пока сетка не заполнена или не истек таймаут (завершить, если сетка заполнена или истекло время)

		вывести полученный макет сетки




		Пояснение основных шагов
		Оценка шаблонов: Каждый раз мы проверяем, какой незаполненный шаблон в сетке имеет 
		наименьшее число возможных вариантов размещения. Это позволяет сначала выбирать самые 
		сложные для заполнения позиции, что помогает создать компактную сетку с максимальным 
		количеством пересечений.

		Контрольные точки (Checkpoint): Если оценка S для шаблона P превышает порог HBound, 
		устанавливаем контрольную точку. Это позволяет вернуться к этой точке, если дальнейшее 
		заполнение приведет к неудаче.

		Выбор слова:

		Если в P еще не размещено слово, выбираем случайное слово из подходящих.
		Если есть еще доступные слова и коэффициент ветвления позволяет, пробуем следующее 
		подходящее слово.
		Если закончились подходящие слова, устанавливаем S = 0, что приведет к откату.
		Условия для отката:

		Если S == 0, оцениваем прогресс.
		Если время превышает TBound, а прогресс меньше PBound, откатываемся к последней 
		контрольной точке.
		Иначе откатываемся к предыдущему узлу (предыдущему слову).
		Размещение слов и обновление пересечений:

		Если слово W можно разместить в P, помещаем его в сетку.
		Обновляем список пересекающихся шаблонов, чтобы все пересекающиеся слова корректно 
		выровнялись.
		Отмечаем шаблон P как заполненный и продолжаем.
		Завершение:

		Цикл продолжается до тех пор, пока сетка не будет полностью заполнена или пока не истечет 
		время. Алгоритм может завершиться раньше, если будет определено, что допустимого размещения 
		в рамках ограничений не существует.
		Такой подход сочетает жадное размещение и откат с контрольными точками, обеспечивая 
		заполнение сетки с максимальным количеством пересечений, насколько это возможно, 
		учитывая заданные слова и их возможные пересечения.
	*/

//Описание Жадного алгоритма
   /*
   * *
	Жадный Алгоритм
	Для жадного алгоритма нужно :
	Выбирать слова, которые покрывают как можно больше непокрытых символов.
	Итеративно добавлять слова, пока не будут покрыты все символы.

	   Используйте Вариант 3 : Матрица пересечений.
	   Подходит для задач, где требуется частая проверка пересечений между парами слов.

	   Описание : Использовать двумерную матрицу, где строки и столбцы представляют
	   слова, а элементы матрицы хранят информацию о пересечениях(например, индексы букв).
	   Реализация :
	   std::vector<std::vector<IntersectionInfo>> intersectionMatrix,
	   где IntersectionInfo описывает пересечения.
	   Преимущества :
	   Простота проверки, есть ли пересечение между двумя словами.
	   Недостатки :
	   Занимает много памяти, особенно если слов много, а пересечений мало.*/
