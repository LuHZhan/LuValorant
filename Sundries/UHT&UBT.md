### 关键要点
- 研究表明，UHT（Unreal Header Tool）在UE5的构建过程中由UBT（Unreal Build Tool）调用，用于解析头文件并生成代码。
- 证据倾向于认为，构建顺序是：UBT启动，调用UHT生成代码，然后编译和链接。
- 这种顺序确保了UObject系统的功能，如反射和蓝图集成，能够在编译前正确设置。

---

### UHT与UBT的区别及编译流程中的配合顺序

#### 概述
UHT和UBT是Unreal Engine 5（UE5）中用于构建和编译项目的两个重要工具。它们在编译流程中各司其职，配合完成从代码解析到最终可执行文件的生成。

#### UHT与UBT的区别
- **UHT（Unreal Header Tool）**：  
  UHT是一个自定义的解析和代码生成工具，专门支持UObject系统。它负责解析C++头文件，处理Unreal特有的宏（如UFUNCTION、UPROPERTY），并生成实现反射、蓝图集成等功能的代码。

- **UBT（Unreal Build Tool）**：  
  UBT是管理整个构建过程的工具，负责协调编译、链接等操作。它根据项目的构建配置，调用各种工具（包括UHT）来确保源代码正确构建。

#### 编译流程中的配合顺序
研究表明，UE5的编译流程大致如下：
1. **UBT启动构建**：UBT根据项目的目标（如Editor、Game）和其他配置开始构建过程。
2. **UBT调用UHT**：UBT调用UHT来解析头文件，生成必要的代码（如`.generated.h`文件），以支持UObject系统的功能。
3. **编译和链接**：UHT生成代码后，UBT使用C++编译器编译源代码（包括生成的代码），然后进行链接，生成最终的可执行文件。

这种顺序确保了UHT在编译前完成代码生成，满足引擎对反射和蓝图集成的需求。

#### 一个意想不到的细节
UHT生成的代码（如`.generated.h`文件）会在后续编译过程中被C++预处理器处理，这与常规宏的展开类似，但UHT的生成步骤是UE5构建系统的一个独特部分。

---

### 详细报告

#### 引言
本文探讨了UE5中UHT和UBT的区别，以及它们在编译流程中的配合顺序。通过分析官方文档和相关资源，我们将详细说明这两个工具的作用及其交互方式，并解释背后的技术原因。

#### UHT与UBT的定义与功能
根据[Unreal Build System | Unreal Engine 4.27 Documentation](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/UnrealBuildSystem/)，UHT和UBT是UE5构建系统的重要组成部分：

- **UBT（Unreal Build Tool）**：  
  UBT是一个自定义工具，管理UE5源代码的构建过程，支持多种构建配置。它读取`.build.cs`和`.target.cs`文件，控制模块的依赖、库路径等，并协调整个编译流程。  
  例如，UBT可以构建Editor目标、Game目标等，并生成Visual Studio项目文件（如`.sln`和`.vcxproj`）以便编辑。

- **UHT（Unreal Header Tool）**：  
  UHT是一个解析和代码生成工具，专门支持UObject系统。它解析C++头文件，识别如UFUNCTION、UPROPERTY等宏，并生成实现反射、序列化、蓝图调用等功能的代码。  
  例如，UHT会生成`.generated.h`文件，包含虚函数表和元数据，用于支持蓝图与C++的交互。

从功能上看，UBT是构建过程的“指挥者”，而UHT是其中的一个“执行者”，专注于头文件处理和代码生成。

#### 编译流程中的配合顺序
研究表明，UE5的编译流程分为多个阶段，UHT和UBT的配合顺序如下：

1. **UBT启动构建**：  
   构建过程由UBT启动，通常通过命令行或IDE（如Visual Studio）触发。UBT根据项目的目标和配置（如Development、Shipping）准备构建环境。  
   例如，运行`RunUBT.bat -Mode=UnrealHeaderTool`会启动UHT模式，表明UBT可以直接调用UHT。

2. **UBT调用UHT**：  
   根据[Extend the Unreal Header Tool with Plugins | Matt’s Game Dev Notebook](https://unrealist.org/uht-plugins/)，UBT会在构建过程中调用UHT来处理头文件。UHT解析C++头文件，生成如`.generated.h`和`.gen.cpp`文件，这些文件包含UObject系统的实现代码。  
   例如，命令`RunUBT.bat -Mode=UnrealHeaderTool -Stats -Json`会执行UHT，并可能触发额外的导出器，生成统计或JSON文件。

3. **编译和链接**：  
   UHT生成代码后，UBT使用C++编译器（如MSVC）编译源代码，包括手动写的代码和UHT生成的代码。编译完成后，UBT进行链接，生成最终的可执行文件（如`.exe`或`.dll`）。  
   根据[Unreal Build System | Unreal Engine 4.27 Documentation](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/UnrealBuildSystem/)，编译分为两个阶段：首先UHT解析和生成代码，然后C++编译器编译结果。

这种顺序的证据来自官方文档，明确指出“Code compilation happens in two phases: UHT is invoked, which parses the C++ headers for Unreal-related class metadata and generates custom code to implement the various UObject-related features. The normal C++ compiler is invoked to compile the results.”（代码编译分为两个阶段：首先调用UHT解析头文件并生成代码，然后调用C++编译器编译结果）。

#### 为什么会有这种顺序
这种顺序的原因在于UHT生成的代码是编译过程的依赖项。例如，UFUNCTION宏需要UHT生成`exec`函数和元数据，以支持蓝图调用C++函数。如果UHT在编译后运行，生成的代码将无法被编译器识别，导致构建失败。

此外，UBT作为主构建工具，负责协调所有步骤，包括调用UHT、编译器和链接器，确保整个流程无缝衔接。

#### 详细比较
以下表格总结了UHT和UBT的区别和作用：

| 特性     | UHT（Unreal Header Tool）                     | UBT（Unreal Build Tool）           |
| -------- | --------------------------------------------- | ---------------------------------- |
| 主要功能 | 解析头文件，生成UObject系统代码               | 管理整个构建过程，包括编译、链接等 |
| 处理对象 | C++头文件，Unreal宏（如UFUNCTION、UPROPERTY） | 源代码、模块依赖、构建配置         |
| 输出     | 生成`.generated.h`和`.gen.cpp`文件            | 生成可执行文件（如`.exe`、`.dll`） |
| 运行时机 | 由UBT在构建过程中调用，编译前                 | 启动并协调整个构建流程             |
| 依赖关系 | 依赖于头文件内容                              | 依赖于UHT生成的代码和源代码        |

#### 一个意想不到的细节
UHT不仅生成代码，还支持扩展插件，如导出器（exporters），可以分析代码并生成统计信息或JSON文件，这在常规C++构建中不常见。例如，[Extend the Unreal Header Tool with Plugins | Matt’s Game Dev Notebook](https://unrealist.org/uht-plugins/)提到，可以通过命令行参数触发额外的导出器，如`-Stats`和`-Json`，这为开发者提供了额外的分析能力。

#### 结论
综上所述，UHT和UBT在UE5的编译流程中各有分工：UHT负责解析头文件并生成代码，UBT则管理整个构建过程并调用UHT。它们的配合顺序是UBT启动，调用UHT生成代码，然后进行编译和链接。这种顺序确保了UObject系统的功能（如反射和蓝图集成）能够正确实现。

---

#### 关键引用
- [Unreal Build System | Unreal Engine 4.27 Documentation](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/UnrealBuildSystem/)
- [UnrealBuildTool | Unreal Engine 4.27 Documentation](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/)
- [Extend the Unreal Header Tool with Plugins | Matt’s Game Dev Notebook](https://unrealist.org/uht-plugins/)