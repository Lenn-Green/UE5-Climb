# Climb - UE5 Hardcore Climbing Demo

这是一个 Unreal Engine 5.4 攀岩游戏 demo 项目，目标是验证一套由 C++ 驱动逻辑、Control Rig 执行身体表现的硬核攀岩模拟框架。

## 项目目标

本项目优先模拟竞技攀岩中的真实身体控制，而不是传统第三人称动作游戏的简化攀爬：

- 身体重心（Center of Mass / Pelvis）决定稳定性和发力空间。
- Body Tension 由接触点、重心偏移和肢体受力共同决定。
- 动量可以在跑墙、摆荡和 Dyno 中转换为跳跃初速度。
- 手、脚、盆骨与墙面的关系由程序驱动，玩家不直接控制离墙 Z 轴。

## 控制方案

- 左摇杆：控制身体重心在平行于墙面的局部 X-Y 平面移动。
- 右摇杆：控制当前激活肢体的墙面探测位置。
- L2 / R2：分别控制左手和右手抓握；按住尝试锁定岩点，松开释放对应手。
- Z 轴自动化：移动手部时盆骨更贴墙；移动脚部时盆骨离墙增加，为换脚和发力留空间。

## 技术架构

项目采用“C++ 是大脑，Control Rig 是脊髓”的分工：

- C++ 负责 Enhanced Input、状态机、Sphere Trace、重心数学、动量、耐力和肌肉张力。
- `AClimbingCharacter` 负责状态编排和输入接入，不承载所有实现细节。
- `UClimbingMovementComponent` 负责攀爬移动模式和运动约束。
- `UClimbingHoldQueryComponent` 负责岩点查询与 Sphere Trace。
- `UClimbingSolver` 负责纯数学计算。
- `UClimbingAnimInstance` 将 C++ 最终目标数据桥接到动画层。
- Control Rig 只负责 FBIK 和表现，不决定抓握、脱手或岩点有效性。

## 当前工程状态

- UE 版本：5.4。
- 工程来源：ThirdPerson 模板，已转换为 C++ 项目。
- 已建立 `Source/Climb` 模块和标准 Public / Private 目录。
- 已实现基础类：`AClimbingCharacter`、`UClimbingMovementComponent`、`UClimbingHoldQueryComponent`、`UClimbingSolver`、`UClimbingAnimInstance`。
- 已配置 Enhanced Input 资产：CoM 移动、肢体探测、左手抓握、右手抓握。
- 已完成基础框架 Phase 0-6，并进入 Playable Loop 1。

## 文档导航

- `AGENTS.md`：AI / Codex 执行工程任务时必须遵守的架构和代码约束。
- `MEMORY.md`：项目长期上下文、当前状态和阶段优先级。
- `docs/GATES.md`：严格门禁系统，定义每个阶段通过标准。
- `docs/IMPLEMENTATION_PLAN.md`：Phase 0-6 基础框架实施计划。
- `docs/PLAYABLE_LOOP_1_PLAN.md`：Playable Loop 1 的原始范围与阶段计划，现已正式闭环。
- `docs/POST_PLAYABLE_LOOP_PLAN.md`：Playable Loop 1 之后的 Rig / 四肢扩展计划。
- `docs/CONTROL_RIG_BRIDGE.md`：AnimInstance 到未来 Control Rig 资产的变量契约。
- `docs/TEST_RECORDS.md`：阶段测试记录，包含命令验证、手动检查、跳过项和风险。
- `.codex/skills/climb-ue5-hardcore-climbing/SKILL.md`：项目本地 Codex skill，用于后续自动加载本项目工作流。

## 下一步

当前优先级是 Playable Loop 1：让角色抓住岩点后真正挂住墙面，左摇杆影响 CoM，右摇杆探测候选岩点，左右手完成基础换手，并输出 Body Tension 调试值。
