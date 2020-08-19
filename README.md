# MiniSQL

数据库作业

## TODO
x 有些注释炸了，使用UTF-8重新完善一下注释
* Mordernize（c++ 17），B+树和Parser不用了
    * 完全摆脱裸指针
    * 能const就const，用override
    * 用array代替数组
    * 使用at而不是[]
    * variant代替union
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

以RAII的方式实现块资源的获取与释放

#### LRU

使用类似stack的方式实现LRU，最近使用的浮到栈顶（其实应该是队列，操作系统PPT里面叫做stack implementation）
