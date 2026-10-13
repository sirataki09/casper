[Project Page](https://yoterel.github.io/casper-project-page/) | [Paper](https://arxiv.org/abs/2409.04397) | [Supplementary](https://yoterel.github.io/casper-project-page/static/pdfs/casper_supp.pdf)

# Casper DPM

This is the official implementation of Casper DPM: Cascaded Perceptual Dynamic Projection Mapping onto Hands

## Installation
### Requirements
- OS: Windows 10 or above
- Visual Studio 19 or above
- CMAKE 3.20 or above
- [OpenCV](https://opencv.org/releases/) 4.6 or above
- A python interpreter 3.9 or above (with dev tools, which is usually included in windows versions), with numpy, mediapipe and nanobind installed:
  `pip install numpy mediapipe nanobind`
- [Gemini Ultra Leap](https://www.ultraleap.com/tracking/gemini-hand-tracking-platform/) - for leap motion controller API (this is in principle optional, but requires some work to seperate dependencies)

Optional:
- [Pylon SDK](https://www.baslerweb.com/en/software/pylon/sdk/) - for basler camera control
- `pip install openai` - if you want casper interactive Projegraphy to work with GPT4.
### How to install
1) Clone the repo (let's assume you cloned into "C:\src\casper"):

  `git clone https://github.com/yoterel/casper.git`
   
2) Change dir:

  `cd C:\src\casper`

3) setup paths in CMakeLists.txt
   This step is manual.
   - set the variable Python_EXECUTABLE to the python executable you will use, for now only a system python was tested and not virtual environments.
   - set the variable OpenCV_DIR to the Opencv install path. Or, place the openCV install path under casper/third-party/opencv.
   - other paths should be detected automatically using environment variables, check error msgs in the next step and replace any missing path.
     
3) configure cmake (this assumes visual studio 2019 is your generator):

  `cmake -SC:/src/casper =BC:/src/casper/build -G "Visual Studio 16 2019" -T host=x64 -A x64`


4) build project:

  `cmake --build C:/src/casper/build --config Release --target ALL_BUILD -j 10 --`

## Usage

Please set the following environment variables before running any of the following commands:

- `PYTHONHOME` should be a path to the python interpreter with numpy and mediapipe installed e.g.:

`set PYTHONHOME=C:/Users/sens/AppData/Local/Programs/Python/Python311`

- `OPENAI_API_KEY` should be set to your openai api key, if you want casper to interact with GPT4 to probe for animals on the fly.

- to run casper in sandbox mode (this will allow you to perform any task that was described in the paper):

  `casper.exe`

Right clicking with the mouse will open a gui with all configurations

- to run a discrete simulation:
  `casper.exe --mode simulation`

To start you can open the menu and check "Debug Playback" under playback settings.

- if you don't have a projector or a camera, but still wish to check out casper, you can run with a simulated camera and projector:
  
  `casper.exe --simcam --simproj`

---

## macOS（Apple Silicon）で画面内シミュレーションを動かす

> この節は公式リポジトリのWindows向け実装をできるだけ維持しつつ、Apple Silicon Macで保存済みデータを再生するために追加した手順です。DynaFlash、Baslerカメラ、MIDIコントローラー、Ultraleap実機は使用しません。実際の手へ投影するモードではなく、C++/OpenGLアプリのウィンドウ内で処理結果を確認するモードです。

### 1. 何が動くか

Mac版の最初の目標は、`resource/recordings` に同梱された手の姿勢データを読み込み、仮想的な手と投影結果をウィンドウに表示することです。

- 使用するもの: CMake、C++、OpenGL、OpenCV、GLFW、Assimp、FreeType、Python、MediaPipe
- 使用しないもの: DynaFlash SDK、Basler Pylon SDK、Windows MIDI API、Ultraleap実機
- Unityは使用しません。このリポジトリ本来のC++アプリをビルドします。

### 2. 対象環境

この手順は次の環境で作成・検証しています。

- Apple Silicon（arm64）Mac
- macOS 26
- Homebrew
- CMake 4.4以降
- Python 3.11
- 仮想環境名: `dm`

Intel Macでは未検証です。

### 3. Homebrew依存関係をインストールする

Homebrewが未導入の場合は、先に[Homebrew公式サイト](https://brew.sh/)の手順で導入してください。その後、ターミナルで次を実行します。

```bash
brew install cmake opencv glfw assimp freetype boost python@3.11
```

OpenCVは多数の関連パッケージを導入するため、完了まで時間とディスク容量を必要とします。

### 4. Python仮想環境 `dm` を作る

リポジトリのルート、つまりこのREADMEがあるディレクトリで実行します。

```bash
cd /Users/takumi/ws/casper
/opt/homebrew/bin/python3.11 -m venv dm
dm/bin/python -m pip install "numpy<2" mediapipe nanobind
```

仮想環境を有効化して作業したい場合は次を実行します。CMakeにはPythonの絶対パスを渡すため、有効化しなくてもビルドできます。

```bash
source dm/bin/activate
```

終了するときは次を実行します。

```bash
deactivate
```

### 5. CMakeを構成する

```bash
cd /Users/takumi/ws/casper
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/opt/homebrew \
  -DPython_EXECUTABLE="$PWD/dm/bin/python"
```

macOSでは `CASPER_MACOS_SIMULATION=ON` が初期値です。Windows専用ハードウェアを誤って有効にしないため、macOSでこの値をOFFにするとCMakeは理由を表示して停止します。

### 6. ビルドする

```bash
cmake --build build --config Release -j 8
```

成功すると実行ファイルは `build/bin/casper` に作られます。

### 7. シミュレーションを起動する

リソースへの相対パスを公式実装と一致させるため、必ず `build/bin` へ移動してから起動します。

```bash
cd /Users/takumi/ws/casper/build/bin
./casper --mode simulation
```

初期状態では、リポジトリに含まれる `resource/recordings/all_10s` を再生します。右クリックで設定GUIを開けます。

### 8. Mac版で問題が起きたときの確認項目

#### `Python` または `NumPy` が見つからない

いったん `build` を残したまま、Pythonの指定を付けてCMakeを再実行します。

```bash
cmake -S . -B build -DPython_EXECUTABLE="$PWD/dm/bin/python" -DCMAKE_PREFIX_PATH=/opt/homebrew
```

#### OpenCV、Assimp、GLFWなどが見つからない

Homebrewの導入状況を確認します。

```bash
brew list --versions opencv glfw assimp freetype boost
```

すべて表示されるのに見つからない場合は、`-DCMAKE_PREFIX_PATH=/opt/homebrew` が付いているか確認してください。

#### 実行直後にリソースが見つからない

`build/bin` 以外から起動すると、公式実装が使用する `../../resource` という相対パスがずれます。次の2行で起動してください。

```bash
cd /Users/takumi/ws/casper/build/bin
./casper --mode simulation
```

### 9. この環境で確認できた結果

2026年10月14日に `/Users/takumi/ws/casper` で次を確認しました。

- CMake構成: 成功
- Releaseビルド: 成功（`build/bin/casper` を生成）
- OpenGLウィンドウ作成: 成功
- GPU: Apple M4
- OpenGL: 4.1 Metal
- GLSL: 4.10
- MediaPipeの手ランドマークモデル初期化: 成功
- `Default.fbx` と `GuessCharGame_palm.fbx` の読込: 成功
- `resource/recordings/all_10s` の骨格・関節・時刻データ読込: 成功
- 骨格のみ録画からの仮想カメラ描画: 成功
- 最初のフレームに黒以外の画素があること: 成功（ログの末尾が `Synthetic simulation frame contains visible pixels: yes`）
- 終了コード: `0`

MediaPipeが出す `INFO` と、`Feedback manager requires a model with a single signature inference` という `W...` 行は、CPU推論バックエンドとモデル機能に関する通知です。Casperの初期化失敗ではありません。`ERROR::`、Python例外、OpenGLエラー、または終了コードが0以外の場合は、この節の成功状態とは異なります。

## 公式リポジトリから変更した点（Mac移植ログ）

元にした公式コミットは `b85af5c2964ab372897601fa7228f7e156048d68` です。既存ファイルは削除していません。Windows向け処理は残し、macOS時だけ別経路を選ぶ方針です。

### ビルドと環境

- `.gitignore`
  - `dm/`、`build-macos/`、`.DS_Store`を追跡対象外に追加しました。
- `CMakeLists.txt`
  - macOSの場合だけ `cmake/macos-simulation.cmake` を読み込みます。
  - 既存のWindows向けCMake定義は、その下に変更せず残しています。
- `cmake/macos-simulation.cmake`（新規）
  - Homebrew版のOpenCV、GLFW、Assimp、FreeType、Boost、zlibを検出します。
  - `dm` のPythonとNumPyをC++アプリへリンクします。
  - Windows専用ソースとSDKをビルド対象から外します。
  - 実行時の相対パスを維持するため、実行ファイルを `build/bin` に出力します。
  - CMake 3.30以降のBoost検出方針（CMP0167）を明示し、古い検出モジュールに関する警告を防ぎます。
  - `dm` の `site-packages` をPython自身から取得し、埋め込みPythonへ渡します。ディレクトリが存在しない場合は構成時に分かりやすく停止します。
  - C++側で実際に使うOpenCVモジュールだけをリンクします。GUI・カメラ入出力用の `highgui` / `videoio` を外し、MediaPipeに同梱されたPython版OpenCVとのmacOSクラス名衝突を避けます。

### Windows専用ハードウェアの代替

- `src/leapCPP_stub.cpp`（新規）
  - Ultraleap SDKへ接続しないスタブです。シミュレーションでは保存済み姿勢を使うため、実機接続待ちで停止しないようにします。
- `src/MidiControllerAPI_stub.cpp`（新規）
  - Windows Multimedia APIを使わず、MIDI APIと同じ関数名を持つ無操作実装です。
- `include/leapCPP.h`
  - Macシミュレーション時の `LeapGetTime()` がWindows/Ultraleapライブラリを要求しないようにしました。
- `include/display.h`、`src/display.cpp`
  - DynaFlash固有の型とWindows APIをWindows時だけコンパイルします。
  - MacではDynaFlashクラスの無操作版を提供し、保存出力用の `SaveToDisk` は元の実装を利用します。
- `include/engine_state.h`
  - Macシミュレーションの初期記録データを、存在しない `test` から同梱済みの `all_10s` に変更しました。Windowsの初期値は元のままです。

### C++・OpenCVのmacOS互換性

- `src/skinned_shader.cpp`
  - Windows専用の文字列整形関数 `_snprintf_s` はWindowsでそのまま使い、macOSでは標準C/C++の `snprintf` を使います。生成するシェーダー変数名は同一です。
- `include/diffuse.h`、`include/post_process.h`
  - OpenCV 5でも `boundingRect`、`convexHull`、`getPerspectiveTransform` などの宣言を確実に読めるよう、画像処理用の `opencv2/imgproc.hpp` と二次元幾何用の `opencv2/geometry/2d.hpp` を明示的に追加しました。
- `include/post_process.h`
  - Clangが警告する不正確な `#endif POSTPROCESS` を、意味が同じ正規のコメント形式 `#endif // POSTPROCESS_H` に直しました。
- `src/user_study.cpp`
  - メンバーは `std::mt19937` 型なのに、コンストラクター内で別型の `std::default_random_engine` を代入していた箇所を、`std::mt19937` のまま乱数シード付きで初期化するよう修正しました。試行順をランダム化する目的は変わりません。
- `src/guess_pose_game.cpp`、`src/guess_char_game.cpp`
  - 同じ乱数器の型不一致を、宣言どおり `std::mt19937` のデフォルト初期化へ修正しました。元コードと同様に固定のデフォルトシードを使います。
- `src/skinned_model.cpp`
  - メッシュ読込前に、それ以前の描画処理が残したOpenGLエラーを番号・対象ファイル名付きで表示して消費します。メッシュ自身の処理後にもエラー番号を表示します。上流実装は直前処理のエラーまで「メッシュ読込失敗」と判定していたため、原因の切り分けと誤判定防止を兼ねています。
- `include/shader.h`
  - シェーダーのコンパイルエラーへ、頂点・フラグメント・ジオメトリの種別だけでなくファイルパスも表示します。多数のシェーダーを順番に読む構成でも、修正対象を特定できます。
- `src/shaders/skin.fs`
  - 構造体名 `Attenuation` と同名だったローカル変数を `attenuationValue` へ変更しました。AppleのGLSLコンパイラでは型名との衝突が構文エラーになるためです。
  - Core Profileで廃止された `texture2D` 呼び出しを、同じサンプリング結果を返す `texture` へ変更しました。
- `src/main.cpp`
  - OpenCV 5でカメラ姿勢推定APIが分離されたため、`Rodrigues`、`solvePnP`、`projectPoints` などを宣言する `opencv2/geometry/3d.hpp` を明示的に追加しました。
  - macOS版では埋め込みPythonの初期化直後に、実行ディレクトリと `dm` の `site-packages` をモジュール検索パスへ追加します。これにより、コピー済みの `predict.py` / `chatgpt.py` と仮想環境内のNumPy / MediaPipeを読み込めます。
  - Macシミュレーションでは使用しないChatGPT/ControlNetクライアントを初期化しません。したがってOpenAIパッケージやAPIキーは、画面内シミュレーションの必須条件ではありません。
  - Apple OpenGL 4.1 Core Profileが保証する範囲に合わせ、macOSだけ線幅を1ピクセルにします。Windowsの5ピクセル設定は維持します。
  - 同梱の `all_10s` には手姿勢の `.npy` だけがあり、カメラ画像PNGはありません。画像がある記録では元の実写再生処理を使い、画像がない記録では既存の仮想カメラ描画処理へ自動的に切り替えます。再生終了後はどちらも先頭へ戻して連続再生します。
  - 骨格のみのMacシミュレーションでは、最初の描画後に画面の画素を一度だけ検査し、黒以外の画素があれば `Synthetic simulation frame contains visible pixels: yes` と表示します。画面表示を自動検証するための診断で、以後のフレームには負荷を加えません。
- `src/diffuse.cpp`
  - 初期化を省略したChatGPTクライアントも安全に破棄できるよう、Pythonオブジェクトのポインターを `nullptr` で初期化し、破棄にはnull対応の `Py_XDECREF` を使用します。

### ビルド中に遭遇したエラーと対処

#### `_snprintf_s` が見つからない

`_snprintf_s` はMicrosoft Cランタイム固有です。macOS側だけ標準関数 `snprintf` を選ぶ条件分岐を追加しました。

#### OpenCVの `boundingRect` / `getPerspectiveTransform` が見つからない

OpenCV 5では幾何処理APIが `opencv2/geometry/2d.hpp` に分離されています。処理内容は変えず、必要な宣言のヘッダーだけを追加しました。

#### `std::mt19937` に別の乱数器を代入できない

上流コードではメンバーの型が `std::mt19937`、代入値が `std::default_random_engine` で一致していませんでした。Clangで型エラーになるため、メンバー宣言どおり `std::mt19937` を `std::random_device` で初期化しました。

#### OpenCVの `Rodrigues` / `solvePnP` / `projectPoints` が見つからない

OpenCV 5では三次元幾何とカメラ姿勢推定APIが `opencv2/geometry/3d.hpp` に分離されています。メイン処理へこのヘッダーを追加し、上流と同じAPI呼び出しを維持しました。

#### 起動時に `No module named 'numpy'` / `No module named 'chatgpt'` と表示される

C++へPythonライブラリをリンクするだけでは、埋め込みPythonは仮想環境を自動的に有効化しませんでした。CMakeが `dm` のパッケージ格納先を取得し、アプリがPython初期化直後にその場所と現在の実行ディレクトリを検索パスへ加えるようにしました。このためREADMEの手順どおり `build/bin` から起動する必要があります。

#### `skin.fs` の183行目でGLSL構文エラーになる

AppleのGLSLコンパイラでは、構造体の型名 `Attenuation` と同名のローカル変数を宣言できませんでした。変数名だけを変え、計算式は維持しています。またCore Profileに合わせて古い `texture2D` を `texture` に置き換えました。

#### `Couldn't import OpenAI` / `Couldn't find OpenAI API key` と表示される

ChatGPTを使った動物推定は別モードの任意機能で、保存済みデータのシミュレーションには不要です。Macシミュレーションビルドではクライアントを初期化しないため、OpenAIパッケージやAPIキーなしで起動できます。

#### `Class CVWindow is implemented in both ...` と表示される

C++アプリがHomebrew版OpenCVの全モジュールをリンクし、MediaPipeがPython wheel同梱版OpenCVを読み込むと、macOS用GUIクラスが二重に登録されます。Mac版CMakeでは必要な `core`、`imgproc`、`imgcodecs`、`video`、`geometry`、`calib` だけをリンクし、競合する `highgui` と `videoio` を読み込まないようにしました。

#### メッシュ読込前にOpenGLエラー `0x501` が表示される

`0x501` は `GL_INVALID_VALUE` です。このMacでは5ピクセルの線幅を指定した時点で発生していました。macOSでは保証される1ピクセルを指定し、Windows側の見た目は変更しない条件分岐にしました。

#### `casper` ウィンドウは開くが真っ黒なまま

同梱の `resource/recordings/all_10s` は、手の骨格・関節・時刻データだけを含み、カメラ画像PNGを含みません。上流の `simulation` 分岐はPNGがある場合にしか描画関数を呼ばないため、ウィンドウだけが黒く表示されていました。Mac版ではPNGがない場合、上流に元からある `playVideo` の仮想カメラ経路で、FBXの手モデルから背景用の手画像と投影結果を生成します。

### OpenGL互換性

- `src/main.cpp`
  - macOSではOpenGL 4.1 Core ProfileとForward Compatibleコンテキストを要求します。
  - Windows固有のスレッドIDを使うNVTX命名処理を、Macシミュレーション時だけ無効化します。
- `src/shaders/grid_texture.vs`
- `src/shaders/grid_texture.fs`
- `src/shaders/grid_color.vs`
- `src/shaders/grid_color.fs`
  - GLSL宣言を4.30から、macOSが提供する4.10へ変更しました。各シェーダーが使う機能は4.10の範囲内です。

### 現時点の制限

- 実際の手、カメラ、プロジェクター、MIDI機器とは接続しません。
- DynaFlashの低遅延投影はWindows版でのみ利用できます。
- Macの一般的なUSBカメラ、Leap Motion、HDMIプロジェクターへの対応は、画面内シミュレーションが安定した後の段階です。
