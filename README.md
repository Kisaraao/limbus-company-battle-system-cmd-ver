# 介绍

> 本项目是纯个人兴趣，模仿月亮计划的边狱巴士做的。
> 
> 使用了pugiXML库读取xml配置文件

按1或2选择技能，然后纯看。

好玩的主要是在自己做角色做技能上。

PS：速度目前只能看看，没实质用处。

目前特效：

> 烧伤：burn
> 
> 流血：bleed
> 
> 破裂：rupture
> 
> 沉沦：sink
> 
> 震颤：tremor
> 
> 呼吸法：breath
> 
> 充能：charge（*可客制化显示名称与颜色*）

> 震颤引爆：tremor-explode（若敌人有混乱条，就消耗一层层数，使敌人最前一个混乱条数值前移 震颤强度 点。（私货：若没有混乱条，则消耗一层层数，直接造成 { 震颤强度 / 2 } 伤害））
> 
> 沉沦泛滥：sink-explode
> 
> 重投：reroll
> 
> （以下特效只填写x）
> 
> 攻击等级提升：attack_level_up
> 
> 攻击等级降低：attack_level_down
> 
> 强壮：strong
> 
> 虚弱：weak
> 
> 伤害强化：damage_enhance
> 
> 伤害弱化：damage_weak
> 
> 守护：protect
> 
> 易伤：fragile

> #### 私货
> 
> burn-explode
> 
> rupture-explode
> 
> bleed-explode

> 可以做到条件触发技能进化
> 
> 可以做到条件触发技能强化
> 攻击类型，罪孽属性也都可客制化 Type.xml 内编写
