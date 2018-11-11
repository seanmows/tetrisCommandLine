#include <iostream>
#include <thread>  
#include <vector>
using namespace std;

#include <Windows.h>

wstring tetrisBlocks[7];
int playFieldWidth = 12;
int playFieldHeight = 18;
unsigned char *pField = nullptr;

//defauly console screen sizes
int screenWidth; 
int screenHeight;




int rotate(int x, int y, int r) {
	switch (r % 4) {
	case 0:	
		return y * 4 + x; //0 deg
		break;
	case 1: 
		return 12 + y - (x * 4); //90 deg
		break;
	case 2: 
		return 15 - (y * 4) - x; //180 deg
		break;
	case 3: 
		return 3 - y + (x * 4);// 270 deg
		break;
	}
	return 0;
}
bool doesPieceFit(int ntetrisBlock, int rotation, int posX, int posY) {
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			//index of peice
			int i = rotate(x, y, rotation);

			//index of field
			int fi = (posY + y) * playFieldWidth + (posX + x);

			if ((posX + x >= 0 && posX + x <= playFieldWidth) && (posY + y >= 0 && posY + y <= playFieldHeight)) {
				if (tetrisBlocks[ntetrisBlock][i] == L'X' && pField[fi] != 0) {
					return false; //block hit object 
				}
			}
		}
	}

	return true;
}
int main()
{	

	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwByteWritten = 0;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(hConsole, &csbiInfo);
	COORD ConsoleWH;
	ConsoleWH = csbiInfo.dwSize;
	screenWidth = ConsoleWH.X; //defualt for my pc 
	screenHeight = ConsoleWH.Y;

	//create blocks
	tetrisBlocks[0].append(L"..X.");
	tetrisBlocks[0].append(L"..X.");
	tetrisBlocks[0].append(L"..X.");
	tetrisBlocks[0].append(L"..X.");

	tetrisBlocks[1].append(L"..X.");
	tetrisBlocks[1].append(L".XX.");
	tetrisBlocks[1].append(L".X..");
	tetrisBlocks[1].append(L"....");

	tetrisBlocks[2].append(L".X..");
	tetrisBlocks[2].append(L".XX.");
	tetrisBlocks[2].append(L"..X.");
	tetrisBlocks[2].append(L"....");

	tetrisBlocks[3].append(L"....");
	tetrisBlocks[3].append(L".XX.");
	tetrisBlocks[3].append(L".XX.");
	tetrisBlocks[3].append(L"....");

	tetrisBlocks[4].append(L"..X.");
	tetrisBlocks[4].append(L".XX.");
	tetrisBlocks[4].append(L".X..");
	tetrisBlocks[4].append(L"....");

	tetrisBlocks[5].append(L"....");
	tetrisBlocks[5].append(L".XX.");
	tetrisBlocks[5].append(L"..X.");
	tetrisBlocks[5].append(L"..X.");

	tetrisBlocks[6].append(L"....");
	tetrisBlocks[6].append(L".XX.");
	tetrisBlocks[6].append(L".X..");
	tetrisBlocks[6].append(L".X..");

	pField = new unsigned char[playFieldWidth*playFieldHeight]; //play feild outline buffer
	for (int x = 0; x < playFieldWidth; x++) {
		for (int y = 0; y < playFieldHeight; y++) {
			pField[y*playFieldWidth + x] = (x == 0 || x == playFieldWidth - 1 || y == playFieldHeight - 1) ? 9 : 0;
		}
	}
	//create screen buffer
	wchar_t *screen = new wchar_t[screenWidth*screenHeight];
	for (int i = 0; i < screenWidth*screenHeight; i++) {
		screen[i] = L' ';
	}



	//basic game logic
	
	bool bGameOver = false;
	
	int currentPiece = 0;
	int currentRotation = 0;
	int currentX = playFieldWidth / 2;
	int currentY = 0;

	bool key[4];
	bool rotateHold = false;

	int speed = 20;
	int speedCount = 0;
	bool blockDown = false;
	int pieceCount = 0;
	int score = 0;
	

	vector<int> compLine;


	while (!bGameOver) {
		//GAME TIMING *******************************************
		this_thread::sleep_for(50ms);
		speedCount++;
		
		blockDown = (speedCount == speed);



		//INPUT ***************************************************
		for (int k = 0; k < 4; k++) {							//R    L  D   U
			key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26"[k]))) != 0; //checks if button is pressed
		}

		//GAME LOGIC **********************************************
		if (key[0] && doesPieceFit(currentPiece, currentRotation, currentX + 1, currentY)) {
				++currentX;
		}
		if (key[1] && doesPieceFit(currentPiece, currentRotation, currentX - 1, currentY)) {
				--currentX;
		}
		if (key[2] && doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
				++currentY;
		}

		//rotate
		if (key[3] && doesPieceFit(currentPiece, currentRotation + 1, currentX, currentY)) {
			if (!rotateHold) {
				++currentRotation;
				rotateHold = true;
			}
		}
		else {
			rotateHold = false;
		}

		if (blockDown) {
			speedCount = 0;
			if (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1)) {
				++currentY;
			}
			else {
				//lock piece into background
				for (int x = 0; x < 4; x++) {
					for (int y = 0; y < 4; y++) {
						if (tetrisBlocks[currentPiece][rotate(x, y, currentRotation)] == L'X') {
							pField[(currentY + y)*playFieldWidth + (currentX + x)] = currentPiece + 1;
						}
					}
				}

				//Increase Difficulty
				pieceCount++;
				if (pieceCount%10 == 0 && pieceCount>10) {
					speed--;
				}


				//check horizontal lines
				for (int y = 0; y < 4; y++) {
					if(currentY + y < playFieldHeight-1){
						bool line = true;
						for (int x = 1; x < playFieldWidth - 1; x++) {
							line &= (pField[(currentY + y)*playFieldWidth + x]) != 0;
						}
						if (line) {
							for (int x = 1; x < playFieldWidth - 1; x++) {
								pField[(currentY + y)*playFieldWidth + x] = 8;
							}
							compLine.push_back(currentY + y);
						}
					}
				}
				//score

				score += 25;
				if (!compLine.empty()) {
					score += (1 << compLine.size()) * 100; //increase score exponentially for each line completed at a time
				}
				//choose next piece
				currentX = playFieldWidth / 2;
				currentY = 0;
				currentRotation = 0;
				currentPiece = rand() % 7;

			bGameOver = !doesPieceFit(currentPiece, currentRotation, currentX, currentY);
			}
		}


		//RENDER OUTPUT **********************************************

		//draw playing field
		for (int x = 0; x < playFieldWidth; x++) {
			for (int y = 0; y < playFieldHeight; y++) {
				screen[(y + 2)*screenWidth + (x+2)] = L" ABCDEFG=#"[pField[y*playFieldWidth + x]];
			}
		}
		
		//draw piece
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				if (tetrisBlocks[currentPiece][rotate(x, y, currentRotation)] == L'X') {
					screen[(currentY + y + 2)*screenWidth + (currentX + x + 2)] = currentPiece + 65; //for ascii characters
				}
			}
		}

		//display score
		swprintf_s(&screen[2 * screenWidth + playFieldWidth + 6], 16, L"Score: %8d", score);

		//make line disappear 
		if (!compLine.empty()) {
			//Display Frame
			WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwByteWritten);
			this_thread::sleep_for(400ms);

			for (auto &v : compLine) {
				for (int x = 1; x < playFieldWidth-1; x++) {
					for (int y = v; y > 0; y--) {
						pField[y*playFieldWidth + x] = pField[(y - 1)*playFieldWidth + x];
						pField[x] = 0;
					}
				}

				compLine.clear();
			}
	}
		
		//Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwByteWritten);
	}
	//end of game
	CloseHandle(hConsole);
	cout << "Nice Try!! Here is your score:" << score << endl;
	system("pause");
	return 0;
}