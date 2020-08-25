# MiniSQL

数据库作业

## In progress

* 有些注释炸了，使用UTF-8重新完善一下注释
* Mordernize（c++ 17），B+树和Parser不用了
    * optional代替了nullable(Done)
    * variant代替union(Done)
    * 在ownership的指导下使用智能指针，尽可能用引用代替非空裸指针
    * 能const就const，用override
    * 使用at而不是[]
    * 用array代替数组
* 操作系统的Page Buffer可不可以用到这里？
* catelog也许不需要用block的方式存储，这样就没有必要受限于4K大小了，catelog可以在内存中常驻。

## How to Build

MSVC和Clang下测试通过

## How to Use

在可执行文件目录下建立一个data文件夹

## 实现列表

* 关键字都是小写的，大写不认为是关键字
* 退出是exit而不是quit

### 表创建删除

```SQL
create table person ( 
    height float,
    pid int,
    name char(32),
    identity char(128) unique,
    primary key(pid)
);

drop table person;
```

## 实现细节

### 文件结构

#### catalog.dat

数据库元信息，第一个block是整个db的信息（DatabaseData）。
这个block中有一个relation数组，第i个relation的元信息（RelationData）在这个文件的第i+1个block

#### relation.dat

第一个block的开头是RelationEntryData，RECORD_START后面是一系列记录。
记录了内碎片（可能也不是内碎片，因为record大小是一样的，所以碎片一定能塞进record）和后续空位.

### Block Manager

#### Block

数据以块的方式存储，一个块4KB

#### Block Guard

以RAII的方式实现块资源的获取与释放. 
BlockManager记录了每个active block的引用计数，类似智能指针。
BlockGuard构造的时候会递增引用计数，析构时会递减引用计数。
在执行LRU的时候，会踢出引用计数为0的块中LRU的那个。

#### LRU

使用类似stack的方式实现LRU，最近使用的浮到栈顶（其实应该是队列，操作系统PPT里面叫做stack implementation）

### Scanner与流式计算

流式计算就是延迟计算。
下面三个scanner是全部创建完，再开始处理数据的，而不是一个scanner处理完之后，第二个scanner处理上一步的结果。
它是一个流水线。

#### DiskScanner

DiskScanner源自From从句。会从From的relation中遍历硬盘上记录（MiniSQL不支持join）。

注意如果有索引存在，遍历会更高效（硬盘上的block难免有内碎片，而索引的遍历每个都是有效的）。
而且的而且，如果我的where从句包含了对这个索引的限定，我可以进一步缩小遍历的范围。

MiniSQL就出于这一点对DiskScanner做了优化，如果where从句全部为AND构成，
那么会从限定中选出第一个带index的field，利用它的index作为迭代器。
（之所以选择第一个，是因为进一步的优化已经不太可能，这取决于对数据分布的研究）

#### FilterScanner

FilterScanner源自Where从句，这没什么好说的，验证是否为true即可。

#### ProjectScanner

源自Select从句，也没什么好说的，构建新的record即可。
