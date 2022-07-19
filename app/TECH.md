# 技术选型

1. DSL选用的lua，基于几个考虑

   * runtime非常小，性能在游戏行业经过验证；
   * 自带的标准库非常迷你，意味着不会有太多无法掌握的运行细节；
   * 扩展方便，lua虚拟机不具备的能力都需要自己用C/C++进行扩展；

2. WHY NOT X?

   1. WHY NOT NODE?

      * Runtime太重，v8基本就20M了；而且node_module的设计非常容易让整个诊断工具的体积失控。

   2. WHY NOT PYTHON?

      * 系统上各种运维命令，是基于python的，需要做环境隔离；

      * 依赖管理麻烦，如果忘记带上某些模块，而目标环境又不可联网，等于制造了新的问题；

# Native模块

1. API文档请自行查询对应C代码，各个函数其实都比较短

```bash
$ wc -l *
      27 helper.h
     154 lavro.cc
      24 lavro.h
     130 lcarray.c
      23 lcarray.h
      46 lerrors.c
      25 lerrors.h
     160 lgsl.c
      25 lgsl.h
      94 lhashlib.c
      24 lhashlib.h
     338 limpala.cc
      24 limpala.h
      35 llimits.c
      24 llimits.h
     328 lmissing.c
      24 lmissing.h
      51 lre2.cc
      24 lre2.h
    1580 total
```

2. 模块备注

| Module  | Description                                                  |
| ------- | ------------------------------------------------------------ |
| missing | lua虚拟机基本不提供操作系统API，这个模块主要用来提供操作系统能力。 |
| Avro    | 提供Avro文件的读取能力，因为主要用途是分析profile，所以没有带其他功能。 |
| gsl     | 提供科学统计能力，详情参考 https://www.gnu.org/software/gsl/doc/html/sort.html#c.gsl_sort 。LICENSE问题考虑，这个一般不会进行静态链接。部署到客户机器上的时候需要执行`sudo yum install -y gsl` |
| hashlib | 提供各种哈希函数，因为不一定用得着，目前仅提供`xxhash`。暂时不考虑将openssl的各种哈希函数包进来。 |
| impala  | 主要提供impala thrift格式的profile解析功能，返回`TRuntimeProfileTree*`，详情参考https://github.com/apache/impala/blob/master/common/thrift/RuntimeProfile.thrift 。 |
| limits  | 定义了各种常量，主要来自于 `<limits.h>` 。                   |
| re2     | lua自带的pattern，功能不如正则表达式强大，参考[Benchmark of Regex Libraries](http://lh3lh3.users.sourceforge.net/reb.shtml) 选择了 [re2](https://github.com/google/re2/)。 |



# Lua模块

1. 请查阅对应lua文件，部分API设计参考Python。