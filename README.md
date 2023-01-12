# SqlDB

This is my final project of DATABASE SYSTEM course (30240262) in Tsinghua University.

清华大学《数据库系统概论》课程大作业

## Usage 运行说明

### Build 编译

项目使用了 antlr 进行语法分析，因此需要预先[下载](https://github.com/antlr/antlr4/tree/master/runtime/Cpp/runtime/src) runtime 库，并将其放在`include/antlr-runtime`目录下（该目录下应当存在`antlr-runtime.h`文件）。
我使用的 antlr 版本为 4.11.1，因此你也应当下载[该版本](https://github.com/antlr/antlr4/releases/tag/4.11.1)的 runtime 库。
你也可以（尝试）使用其他版本的 antlr，语法文件为`src/parser/SQL.g4`。不过可能需要你对`src/parser/visitor.cpp`进行适配性改动。

```bash
cmake -S . -B build
cmake --build build
```

编译后的可执行文件应当位于 `build/sqldb`，你可以将其移动到所需的工作目录。

### Run 运行

```bash
./sqldb [<default_db>]
```

`<default_db>` 为可选参数，指定默认数据库的名称（即执行`USE <default_db>`）。
