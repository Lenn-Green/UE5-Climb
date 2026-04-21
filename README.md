# Climb - UE5 Hardcore Climbing Demo

这是一个 Unreal Engine 5.4 攀岩游戏 demo 项目，目标是验证一套由 C++ 驱动逻辑、Control Rig 执行身体表现的硬核攀岩模拟框架。

## 项目目标

本项目优先模拟竞技攀岩中的真实身体控制，而不是传统第三人称动作游戏的简化攀爬：

- 身体重心（Center of Mass / Pelvis）决定稳定性和发力空间。
- 动量可以在跑墙、摆荡和 Dyno 中转换为跳跃初速度。
- Body Tension 用接触点、重心偏移和肢体受力来判断姿态是否稳定。
- 手、脚、盆骨距离墙面的关系由程序驱动，玩家不直接控制离墙 Z 轴。

## 控制方案

- 左摇杆：控制身体重心在平行于墙面的 X-Y 平面移动。
- 右摇杆：控制当前激活肢体的墙面探测位置。
- L2 / R2：分别控制左手和右手抓握。按住激活对应肢体控制，松开进入脱手物理。
- Z 轴自动化：移动手部时盆骨更贴墙；移动脚部时盆骨离墙增加，为换脚和发力留空间。

## 技术架构

项目采用“C++ 是大脑，Control Rig 是脊髓”的分工：

- C++ 负责 Enhanced Input、状态机、Sphere Trace、重心数学、动量、耐力和肌肉张力。
- Control Rig 负责 Full Body IK 的最终解析。
- AnimInstance 每帧把 C++ 计算好的 Effector Targets 和身体偏移推送给 Control Rig 变量。

## 当前工程状态

- 当前工程是 UE 5.4 项目。
- 工程仍基于 ThirdPerson Blueprint 模板。
- `Config/DefaultInput.ini` 已配置 Enhanced Input 的默认输入类。
- 当前尚未创建 `Source/` C++ 模块。
- 当前阶段是基础框架：角色基类、Enhanced Input 映射、岩点 Sphere Trace。

## 文档导航

- `AGENTS.md`：AI / Codex 执行工程任务时必须遵守的架构和代码约束。
- `MEMORY.md`：项目长期上下文、当前状态和阶段优先级。
- `docs/GATES.md`：严格门禁系统，定义每个阶段通过标准。
- `docs/IMPLEMENTATION_PLAN.md`：基础框架的分阶段实施计划。
- `.codex/skills/climb-ue5-hardcore-climbing/SKILL.md`：项目本地 Codex skill，用于后续自动加载本项目工作流。

## 下一步

下一阶段应先创建 C++ 模块和标准目录骨架，再实现 `AClimbingCharacter`、`UClimbingMovementComponent`、Enhanced Input 动作绑定，以及独立封装的岩点 Sphere Trace 查询。
