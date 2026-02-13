#include <Novice.h>
#include<thread>
#include<queue>
#include<mutex>
#include<fstream>
#include<sstream>
#include<map>
#include<assert.h>

const char kWindowTitle[] = "LE2D_24";


enum MapChipType
{
	NONE,
	GROUND,
};

namespace
{
	std::map<std::string, MapChipType> mapChipTable =
	{
		{"0",MapChipType::NONE},
		{"1",MapChipType::GROUND},
	};
}

// マップチップ
static inline const uint32_t kNumBlockVirtical = 23;
static inline const uint32_t kNumBlockHorizontal = 40;
int map[kNumBlockVirtical][kNumBlockHorizontal] = {};

// ファイル読み込み関数
void LoadMapChipCsv(const std::string& filePath)
{
	// ファイル読み込み
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	std::stringstream mapChipCsv;
	mapChipCsv << file.rdbuf();
	file.close();

	for (uint32_t i = 0; i < kNumBlockVirtical; ++i)
	{
		std::string line;
		getline(mapChipCsv, line);
		std::istringstream line_stream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j)
		{
			std::string word;
			getline(line_stream, word, ',');

			if (mapChipTable.contains(word))
			{
				map[i][j] = mapChipTable[word];
			}
		}
	}
}






// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };



	// ブロックの大きさ
	int blockSize = 32;

	// スレッド関連
	std::mutex mutex;
	std::condition_variable condition;
	std::queue<int> q;
	bool exit = false;

	// バックグラウンドループ
	std::thread th(
		[&](){
			while (!exit){
				int data;
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));{
					std::unique_lock<std::mutex> uniqueLock(mutex);
					condition.wait(uniqueLock, [&]() { return !q.empty() || exit; });
					if (exit)	{
						break;
					}
					data = q.front();
					q.pop();
				}
				if (data == 1)	{
					LoadMapChipCsv("map.csv");
				}
			}
		}
	);




	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0)
	{
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///
		/// 
		
		if (keys[DIK_SPACE])
		{
			std::unique_lock<std::mutex> uniqueLock(mutex);
			q.push(1);
			condition.notify_all();
		}

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		//画
		for (int y = 0; y < kNumBlockVirtical; y++)
		{
			for (int x = 0; x < kNumBlockHorizontal; x++)
			{
				if (map[y][x] == GROUND)
				{
					Novice::DrawBox(x * blockSize, y * blockSize, blockSize, blockSize, 0.0f, 0x000000FF, kFillModeSolid);
				}
			}
		}

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0)
		{
			break;
		}
	}

	exit = true;
	th.join();


	// ライブラリの終了
	Novice::Finalize();
	return 0;
}