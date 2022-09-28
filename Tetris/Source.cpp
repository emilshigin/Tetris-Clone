#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>
using namespace std;


wstring tetromino[7];				// Tetris Pices
int nFieldWidth = 12;				// Board Width
int nFieldHeight = 18;				// Board Hieght
unsigned char* pField = nullptr;	

int nScreenWidth = 120;				// Console Width
int nScreenHeight = 30;				// Console Height

int Rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0: return py * 4 + px;			// 0 degrees 
	case 1: return 12 + py - (px * 4);	// 90 degrees 
	case 2: return 15 - (py * 4) - px;	// 180 degrees 
	case 3: return 3 - py + (px * 4);	// 270 degrees 
	}
	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			//Get index into piece
			int pi = Rotate(px, py, nRotation);

			//Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			//Collition Detection
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					if(tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false;
				}

			}
		}

	return true;
}

int main()
{
	//create assets
	tetromino[0].append(L"..X...X...X...X.");
	tetromino[1].append(L"..X..XX..X......");
	tetromino[2].append(L".X...XX...X.....");
	tetromino[3].append(L".....XX..XX.....");
	tetromino[4].append(L"..X..XX...X.....");
	tetromino[5].append(L".....XX...X...X.");
	tetromino[6].append(L".....XX..X...X..");

	// Inizalize Playing Field and Border
	pField = new unsigned char[nFieldWidth * nFieldHeight];
	for (int x = 0; x < nFieldWidth; x++) 
		for (int y = 0; y < nFieldHeight; y++)	
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	// Terminal as screen buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

		//Resize Window base on screen hight
	// _SMALL_RECT Rect;
	// Rect.Top = 0;
	// Rect.Left = 0;
	// Rect.Bottom = nScreenHeight -1;
	// Rect.Right = nScreenWidth -1;
	// SetConsoleWindowInfo(hConsole, TRUE, &Rect);



	bool bGameOver = false;

	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bKey[4];
	bool bHoldRotateKey = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	int nPieceCount = 0;
	int nScore = 0;

	vector<int> vLines;

	while (!bGameOver)
	{
		// Game Timing ====================================
		this_thread::sleep_for(50ms);//Game Tick
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		// Input ==========================================
		for (int k = 0; k < 4; k++)					//Arrow Keys   R    L   D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		// [TODO] ADD keys Left Right Rotate
		// Game Logic =====================================

		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
		if (bKey[3])
		{
			nCurrentRotation += (!bHoldRotateKey && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bHoldRotateKey = true;
		}
		else {
			bHoldRotateKey = false;
		}


		// Rotate Left
		//nCurrentRotation -= (bKey[4] && DoesPieceFit(nCurrentPiece, nCurrentRotation - 1, nCurrentX, nCurrentY)) ? 1 : 0;

		//Game Logic
		if (bForceDown)
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++;
			else
			{
				// Lock Current Piece Into Field
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				nPieceCount++;
				if (nPieceCount % 10 == 0 && nSpeed >= 10)
					nSpeed--;


				// Check Have Any Rows Completed
				// First check that you are checking in bounds
				// Is there any lines filled
				// If true indicate with turning row =
				// then Delete the line
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						// Check if line filled
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						// If filled, set line to =
						if (bLine)
						{
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;

							vLines.push_back(nCurrentY + py);
						}
					}

				nScore += 25;
				if (!vLines.empty()) 
					nScore += (1 << vLines.size()) * 100;

				// Choose Next Piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// Game Over State
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nSpeedCounter = 0;
		}
		// Render Output ==================================
		// Draw Field 
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

		// Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65; 

		// Draw Score 
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"Score: %8d", nScore);

		if (!vLines.empty())
		{
			//Display Frame 
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms);

			for (auto &v : vLines)
				for ( int px = 1; px < nFieldWidth -1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}

			vLines.clear();
		}

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	CloseHandle(hConsole);
	cout << "Game Over Your Score: " << nScore << endl;
	system("pause");

	return 0;
}