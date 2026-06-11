# mygrep

C++23を利用したシンプルな`grep`コマンド風ツール

A simple `grep`-like utility written in C++23.

---

## 概要

mygrep は UNIX の `grep` コマンドを参考に作成した、文字列や正規表現を検索するコマンドラインツールです。

このプログラムは作者が進めているC++学習プロジェクト群の一環として作成されました。

---

## 特徴

* 文字列検索
* 拡張正規表現検索 (`-E`)
* 大文字小文字を無視した検索 (`-i`)
* 非一致行の検索 (`-v`)
* 行番号表示 (`-n`)
* マッチ行数の集計 (`-c`)
* 複数ファイルの同時処理
* 標準入力の処理
* `--help`
* `--version`

---

## 動作環境

* C++23 対応コンパイラ
* CMake 4.1 以降

開発・動作確認環境:

* GCC
* WSL (Windows Subsystem for Linux)

---

## ビルド

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

---

## 使用方法

```bash
mygrep [OPTION]... PATTERN [FILE]...
```

例:

```bash
mygrep hello sample.txt

mygrep -inE "(foo|bar)" sample.txt

mygrep -c hello sample.txt

mygrep file1.txt file2.txt
```

標準入力:

```bash
cat sample.txt | mygrep hello
```

---

## オプション

| Option                    | Description |
| ------------------------- | ----------- |
| `-n`, `--line-number`     | 行番号を表示      |
| `-v`, `--invert-match`    | 非一致行を選択     |
| `-i`, `--ignore-case`     | 大文字小文字を無視   |
| `-E`, `--extended-regexp` | 拡張正規表現を使用   |
| `-c`, `--count`           | マッチした行数のみ表示 |
| `--help`                  | ヘルプを表示      |
| `-V`, `--version`         | バージョン情報を表示  |

---

## 実装について

このプロジェクトでは以下の学習を目的としています。

* コマンドライン引数解析
* `std::getline`
* `std::regex`
* 文字列検索
* ファイル入出力
* エラーハンドリング
* CMake によるビルド管理

---

## 制限事項

* GNU grep の完全互換実装ではありません。
* GNU grep とオプション仕様や出力形式が異なる場合があります。
* バイナリファイルの処理は想定していません。
* 高度な正規表現機能やGNU拡張には対応していません。

---

## 開発状況

現在のバージョンは一通りの機能実装を完了した状態です。

今後は不具合修正やリファクタリングを除き、大きな機能追加の予定はありません。

---

## バージョン

```text
mygrep 2.0
```

---

## ライセンス

このプロジェクトはMITライセンスに基づきライセンスされています．
詳細はLICENSEファイルを確認してください．

---

## 関連プロジェクト

本プロジェクトは C++ 学習プロジェクト群の一環として開発されています。

他のトレーニングプロジェクトについては以下を参照してください。

| プロジェクト | 状況 | リンク                                     | 概要             |
| ------ | -- | --------------------------------------- | -------------- |
| mywc   | 完了 | https://github.com/Dize-Azki6674/mywc   | wc 風テキスト統計ツール  |
| mygrep | 完了 | https://github.com/Dize-Azki6674/mygrep | grep 風文字列検索ツール |

---

## 作者

Azkey

---

## Overview

mygrep is a simple command-line utility inspired by the UNIX `grep` command.

It searches text files and standard input for matching strings or regular expressions.

This project is part of a personal C++ training series.

---

## Features

* Plain string search
* Extended regular expression search (`-E`)
* Case-insensitive search (`-i`)
* Inverted matching (`-v`)
* Line number display (`-n`)
* Match count display (`-c`)
* Multiple file support
* Standard input support
* `--help`
* `--version`

---

## Requirements

* C++23 compatible compiler
* CMake 4.1 or newer

Tested with:

* GCC
* WSL (Windows Subsystem for Linux)

---

## Build

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

---

## Usage

```bash
mygrep [OPTION]... PATTERN [FILE]...
```

Examples:

```bash
mygrep hello sample.txt

mygrep -inE "(foo|bar)" sample.txt

mygrep -c hello sample.txt

mygrep hello file1.txt file2.txt
```

Standard input:

```bash
cat sample.txt | mygrep hello
```

---

## Options

| Option                    | Description                             |
| ------------------------- | --------------------------------------- |
| `-n`, `--line-number`     | Print line numbers                      |
| `-v`, `--invert-match`    | Select non-matching lines               |
| `-i`, `--ignore-case`     | Ignore case distinctions                |
| `-E`, `--extended-regexp` | Use extended regular expressions        |
| `-c`, `--count`           | Print only the number of matching lines |
| `--help`                  | Display help                            |
| `-V`, `--version`         | Display version information             |

---

## Implementation Notes

This project was created for learning modern C++.

Topics practiced in this project include:

* Command-line argument parsing
* `std::regex`
* `std::getline`
* String search
* File I/O
* Error handling
* Build management with CMake

---

## Limitations

* This is not a fully compatible GNU grep implementation.
* Option behavior and output formatting may differ from GNU grep.
* Binary file processing is not a target use case.
* Advanced GNU-specific regular expression features are not supported.

---

## Development Status

The current version has completed the implementation of all essential features.

Going forward, there are no plans for major feature additions other than bug fixes and refactoring.

---

## Version

```text
mygrep 2.0
```

---

## License

This project is licensed under the MIT License.
See the LICENSE file for details.

---

## See Also

This project is part of a series of C++ training projects.

The current status of other projects in the training series can be found below.

| Project | Status    | Link                                    | Description                       |
| ------- | --------- | --------------------------------------- | --------------------------------- |
| mywc    | Completed | https://github.com/Dize-Azki6674/mywc   | A wc-like text statistics utility |
| mygrep  | Completed | https://github.com/Dize-Azki6674/mygrep | A grep-like string search utility |

---

## Author

Azkey
