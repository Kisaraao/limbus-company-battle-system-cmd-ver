#pragma once
#include <vector>
#include <unordered_map>
#include <functional>

// 1. 定义事件类型（本质就是个标签）
enum class BattleEvent {
    TurnStart,      // 回合开始
    BeforeCombat,   // 拼点前
    RollCoin,       // 投掷硬币
    AfterCombat,   // 拼点后
    BeforeDamage,   // 攻击前
    Damage,         // 攻击中
    Critical,       // 暴击
    AfterDamage,    // 攻击后
    TurnEnd,        // 回合结束
};

// 2. 定义回调函数类型（函数指针的现代写法）
using EventCallback = std::function<void(void*)>;
// void* data 是个万能口袋，可以传任何数据的地址进去
// 监听器自己负责把 void* 转成具体类型

// 3. 事件总线类（全局消息中转站）
class EventBus {
private:
    // 核心结构：每个事件对应一个回调函数列表
    std::unordered_map<BattleEvent, std::vector<EventCallback>> listeners;

public:
    // 单例模式：保证全局只有一个总线
    static EventBus& get() {
        static EventBus instance;
        return instance;
    }

    // 4. 订阅：告诉总线“当XX事件发生时，请调用我这个函数”
    void subscribe(BattleEvent event, EventCallback callback) {
        listeners[event].push_back(callback);
        // 示例：listeners[TurnEnd] 里可能存了[扣血函数, 烧伤结算函数, 被动检查函数...]
    }

    // 5. 发布：当事件发生时，调用所有订阅了这个事件的函数
    void dispatch(BattleEvent event, void* data = nullptr) {
        // 找到这个事件的所有监听器
        auto it = listeners.find(event);
        if (it == listeners.end()) return; // 没人监听这个事件

        // 遍历所有回调函数，逐个调用
        for (auto& callback : it->second) {
            callback(data); // 把数据口袋传进去
        }
    }
};

//struct DamageEventData {
//    BattleCharacter* attacker;
//    BattleCharacter* target;
//    Coin* coin;
//};