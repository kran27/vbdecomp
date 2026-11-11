#include "GameAnimationCache.hpp"
#include "graphics/AnimationObject.hpp"
#include "configuration/ResourceConfigurationRead.hpp"
#include "graphics/StringFormat.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace JE
{
	void __cdecl LogDebugString(const char* format, ...);
	//45D783/45D786 retain argument cleanup and continuation after48CC40.
	void __cdecl FatalError(const char* format, ...);
}

namespace F3
{
	// Original string table0x6F4170, initialized from the literal strings by
	// 0x63E340 and following code. IDs are retained; no toolkit idle heuristic.
	std::string g_animationGroups[516] =
	{
		"", // 0 0x63e35c
		"_Climb_Una_DmgBk", // 1 0x63e39a
		"_Climb_Una_DmgBkMult", // 2 0x63e3cd
		"_Climb_Una_Down", // 3 0x63e400
		"_Climb_Una_Down_to_Def", // 4 0x63e43a
		"_Climb_Una_Up", // 5 0x63e46d
		"_Climb_Una_Up_to_Def", // 6 0x63e4a0
		"_Com_1H_Attack1", // 7 0x63e4da
		"_Com_1H_Attack2", // 8 0x63e50d
		"_Com_1H_Attack3", // 9 0x63e540
		"_Com_1H_Attack4", // 10 0x63e57a
		"_Com_1H_Gun_Burst", // 11 0x63e5ad
		"_Com_1H_Celebrate", // 12 0x63e5e0
		"_Com_1H_Corner_Gun_Burst", // 13 0x63e61a
		"_Com_1H_Corner_Gun_NoRecoil", // 14 0x63e64d
		"_Com_1H_Corner_Gun_Shoot", // 15 0x63e680
		"_Com_1H_Corner_Gun_Sweep", // 16 0x63e6ba
		"_Com_1H_DmgBk", // 17 0x63e6ed
		"_Com_1H_DmgBkMult", // 18 0x63e720
		"_Com_1H_DmgFr", // 19 0x63e75a
		"_Com_1H_DmgFrMult", // 20 0x63e78d
		"_Com_1H_Death", // 21 0x63e7c0
		"_Com_1H_Dodge", // 22 0x63e7fa
		"_Com_1H_Equip", // 23 0x63e82d
		"_Com_1H_Gun_Duel_Butt", // 24 0x63e860
		"_Com_1H_Switch_Weapons", // 25 0x63e89a
		"_Com_1H_Idle", // 26 0x63e8cd
		"_Com_1H_Knock_ProneB", // 27 0x63e900
		"_Com_1H_Knock_ProneF", // 28 0x63e93a
		"_Com_1H_Limp", // 29 0x63e96d
		"_Com_1H_Limp_DmgBk", // 30 0x63e9a0
		"_Com_1H_Limp_DmgFr", // 31 0x63e9da
		"_Com_1H_Maintenance", // 32 0x63ea0d
		"_Com_1H_Gun_NoRecoil", // 33 0x63ea40
		"_Com_1H_Parallel_Gun_Burst", // 34 0x63ea7a
		"_Com_1H_Parallel_Gun_NoRecoil", // 35 0x63eaad
		"_Com_1H_Parallel_Gun_Shoot", // 36 0x63eae0
		"_Com_1H_Parallel_Gun_Sweep", // 37 0x63eb1a
		"_Com_1H_Parallel_Melee_Slash", // 38 0x63eb4d
		"_Com_1H_Parallel_Melee_Thrust", // 39 0x63eb80
		"_Com_1H_Run", // 40 0x63ebba
		"_Com_1H_Run_DmgBk", // 41 0x63ebed
		"_Com_1H_Run_DmgFr", // 42 0x63ec20
		"_Com_1H_Gun_Shoot", // 43 0x63ec5a
		"_Com_1H_Side_Gun_Burst", // 44 0x63ec8d
		"_Com_1H_Side_Gun_NoRecoil", // 45 0x63ecc0
		"_Com_1H_Side_Gun_Shoot", // 46 0x63ecfa
		"_Com_1H_Side_Gun_Sweep", // 47 0x63ed09
		"_Com_1H_Side_Melee_Slash", // 48 0x63ed18
		"_Com_1H_Side_Melee_Thrust", // 49 0x63ed27
		"_Com_1H_Melee_Slash", // 50 0x63ed36
		"_Com_1H_Stunned", // 51 0x63ed45
		"_Com_1H_Gun_Sweep", // 52 0x63ed54
		"_Com_1H_Throw", // 53 0x63ed63
		"_Com_1H_Melee_Thrust", // 54 0x63ed72
		"_Com_1H_to_ComFly", // 55 0x63ed81
		"_Com_1H_to_Def", // 56 0x63ed90
		"_Com_1H_to_Limp", // 57 0x63ed9f
		"_Com_1H_to_Run", // 58 0x63edae
		"_Com_1H_to_Walk", // 59 0x63edbd
		"_Com_1H_Unequip", // 60 0x63edcc
		"_Com_1H_Walk", // 61 0x63eddb
		"_Com_1H_Walk_DmgBk", // 62 0x63edea
		"_Com_1H_Walk_DmgFr", // 63 0x63edf9
		"_Com_1H_Gun_Butt", // 64 0x63ee08
		"_Com_1H_Laydown_ProneB", // 65 0x63ee17
		"_Com_1H_Laydown_ProneF", // 66 0x63ee26
		"_Com_2HC_Celebrate", // 67 0x63ee35
		"_Com_2HC_DmgBk", // 68 0x63ee44
		"_Com_2HC_DmgBkMult", // 69 0x63ee53
		"_Com_2HC_DmgFr", // 70 0x63ee62
		"_Com_2HC_DmgFrMult", // 71 0x63ee71
		"_Com_2HC_Dodge", // 72 0x63ee80
		"_Com_2HC_Equip", // 73 0x63ee8f
		"_Com_2HC_Idle", // 74 0x63ee9e
		"_Com_2HC_Knock_ProneB", // 75 0x63eead
		"_Com_2HC_Knock_ProneF", // 76 0x63eebc
		"_Com_2HC_Limp", // 77 0x63eecb
		"_Com_2HC_Limp_DmgBk", // 78 0x63eeda
		"_Com_2HC_Limp_DmgFr", // 79 0x63eee9
		"_Com_2HC_Maintenance", // 80 0x63eef8
		"_Com_2HC_Run", // 81 0x63ef07
		"_Com_2HC_Run_DmgBk", // 82 0x63ef16
		"_Com_2HC_Run_DmgFr", // 83 0x63ef25
		"_Com_2HC_Melee_Slash", // 84 0x63ef34
		"_Com_2HC_Stunned", // 85 0x63ef43
		"_Com_2HC_Melee_Thrust", // 86 0x63ef52
		"_Com_2HC_to_Def", // 87 0x63ef61
		"_Com_2HC_to_Limp", // 88 0x63ef70
		"_Com_2HC_to_Run", // 89 0x63ef7f
		"_Com_2HC_to_Walk", // 90 0x63ef8e
		"_Com_2HC_Unequip", // 91 0x63ef9d
		"_Com_2HC_Walk", // 92 0x63efac
		"_Com_2HC_Walk_DmgBk", // 93 0x63efbb
		"_Com_2HC_Walk_DmgFr", // 94 0x63efca
		"_Com_2HC_Laydown_ProneB", // 95 0x63efd9
		"_Com_2HC_Laydown_ProneF", // 96 0x63efe8
		"_Com_2HH_Gun_Burst", // 97 0x63eff7
		"_Com_2HH_Celebrate", // 98 0x63f006
		"_Com_2HH_DmgBk", // 99 0x63f015
		"_Com_2HH_DmgBkMult", // 100 0x63f024
		"_Com_2HH_DmgFr", // 101 0x63f033
		"_Com_2HH_DmgFrMult", // 102 0x63f042
		"_Com_2HH_Dodge", // 103 0x63f051
		"_Com_2HH_Equip", // 104 0x63f060
		"_Com_2HH_Idle", // 105 0x63f06f
		"_Com_2HH_Knock_ProneB", // 106 0x63f07e
		"_Com_2HH_Knock_ProneF", // 107 0x63f08d
		"_Com_2HH_Limp", // 108 0x63f09c
		"_Com_2HH_Limp_DmgBk", // 109 0x63f0ab
		"_Com_2HH_Limp_DmgFr", // 110 0x63f0ba
		"_Com_2HH_Maintenance", // 111 0x63f0c9
		"_Com_2HH_Gun_NoRecoil", // 112 0x63f0d8
		"_Com_2HH_Run", // 113 0x63f0e7
		"_Com_2HH_Run_DmgBk", // 114 0x63f0f6
		"_Com_2HH_Run_DmgFr", // 115 0x63f105
		"_Com_2HH_Gun_Shoot", // 116 0x63f114
		"_Com_2HH_Stunned", // 117 0x63f123
		"_Com_2HH_Gun_Sweep", // 118 0x63f132
		"_Com_2HH_to_Def", // 119 0x63f141
		"_Com_2HH_to_Limp", // 120 0x63f150
		"_Com_2HH_to_Run", // 121 0x63f15f
		"_Com_2HH_to_Walk", // 122 0x63f16e
		"_Com_2HH_Unequip", // 123 0x63f17d
		"_Com_2HH_Walk", // 124 0x63f18c
		"_Com_2HH_Walk_DmgBk", // 125 0x63f19b
		"_Com_2HH_Walk_DmgFr", // 126 0x63f1aa
		"_Com_2HH_Laydown_ProneB", // 127 0x63f1b9
		"_Com_2HH_Laydown_ProneF", // 128 0x63f1c8
		"_Com_2HO_Celebrate", // 129 0x63f1d7
		"_Com_2HO_DmgBk", // 130 0x63f1e6
		"_Com_2HO_DmgBkMult", // 131 0x63f1f5
		"_Com_2HO_DmgFr", // 132 0x63f204
		"_Com_2HO_DmgFrMult", // 133 0x63f213
		"_Com_2HO_Dodge", // 134 0x63f222
		"_Com_2HO_Equip", // 135 0x63f231
		"_Com_2HO_Idle", // 136 0x63f240
		"_Com_2HO_Knock_ProneB", // 137 0x63f24f
		"_Com_2HO_Knock_ProneF", // 138 0x63f25e
		"_Com_2HO_Limp", // 139 0x63f26d
		"_Com_2HO_Limp_DmgBk", // 140 0x63f27c
		"_Com_2HO_Limp_DmgFr", // 141 0x63f28b
		"_Com_2HO_Maintenance", // 142 0x63f29a
		"_Com_2HO_Run", // 143 0x63f2a9
		"_Com_2HO_Run_DmgBk", // 144 0x63f2b8
		"_Com_2HO_Run_DmgFr", // 145 0x63f2c7
		"_Com_2HO_Melee_Slash", // 146 0x63f2d6
		"_Com_2HO_Stunned", // 147 0x63f2e5
		"_Com_2HO_Throw", // 148 0x63f2f4
		"_Com_2HO_Melee_Thrust", // 149 0x63f303
		"_Com_2HO_to_Def", // 150 0x63f312
		"_Com_2HO_to_Limp", // 151 0x63f321
		"_Com_2HO_to_Run", // 152 0x63f330
		"_Com_2HO_to_Walk", // 153 0x63f33f
		"_Com_2HO_Unequip", // 154 0x63f34e
		"_Com_2HO_Walk", // 155 0x63f35d
		"_Com_2HO_Walk_DmgBk", // 156 0x63f36c
		"_Com_2HO_Walk_DmgFr", // 157 0x63f37b
		"_Com_2HO_Laydown_ProneB", // 158 0x63f38a
		"_Com_2HO_Laydown_ProneF", // 159 0x63f399
		"_Com_2HR_Gun_Rifle_Burst", // 160 0x63f3a8
		"_Com_2HR_Celebrate", // 161 0x63f3b7
		"_Com_2HR_DmgBk", // 162 0x63f3c6
		"_Com_2HR_DmgBkMult", // 163 0x63f3d5
		"_Com_2HR_DmgFr", // 164 0x63f3e4
		"_Com_2HR_DmgFrMult", // 165 0x63f3f3
		"_Com_2HR_Dodge", // 166 0x63f402
		"_Com_2HR_Equip", // 167 0x63f411
		"_Com_2HR_Gun_SubMac_Burst", // 168 0x63f420
		"_Com_2HR_Gun_SubMac_NoRecoil", // 169 0x63f42f
		"_Com_2HR_Gun_SubMac_Shoot", // 170 0x63f43e
		"_Com_2HR_Gun_SubMac_Sweep", // 171 0x63f44d
		"_Com_2HR_Idle", // 172 0x63f45c
		"_Com_2HR_Knock_ProneB", // 173 0x63f46b
		"_Com_2HR_Knock_ProneF", // 174 0x63f47a
		"_Com_2HR_Limp", // 175 0x63f489
		"_Com_2HR_Limp_DmgBk", // 176 0x63f498
		"_Com_2HR_Limp_DmgFr", // 177 0x63f4a7
		"_Com_2HR_Maintenance", // 178 0x63f4b6
		"_Com_2HR_Gun_Rifle_NoRecoil", // 179 0x63f4c5
		"_Com_2HR_Run", // 180 0x63f4d4
		"_Com_2HR_Run_DmgBk", // 181 0x63f4e3
		"_Com_2HR_Run_DmgFr", // 182 0x63f4f2
		"_Com_2HR_Gun_Rifle_Shoot", // 183 0x63f501
		"_Com_2HR_Stunned", // 184 0x63f510
		"_Com_2HR_Gun_Rifle_Sweep", // 185 0x63f51f
		"_Com_2HR_to_Default", // 186 0x63f52e
		"_Com_2HR_to_Limp", // 187 0x63f53d
		"_Com_2HR_to_Run", // 188 0x63f54c
		"_Com_2HR_to_Walk", // 189 0x63f55b
		"_Com_2HR_Unequip", // 190 0x63f56a
		"_Com_2HR_Walk", // 191 0x63f579
		"_Com_2HR_Walk_DmgBk", // 192 0x63f588
		"_Com_2HR_Walk_DmgFr", // 193 0x63f597
		"_Com_2HR_Gun_Butt", // 194 0x63f5a6
		"_Com_2HR_Laydown_ProneB", // 195 0x63f5b5
		"_Com_2HR_Laydown_ProneF", // 196 0x63f5c4
		"_Com_2HS_Celebrate", // 197 0x63f5d3
		"_Com_2HS_DmgBk", // 198 0x63f5e2
		"_Com_2HS_DmgBkMult", // 199 0x63f5f1
		"_Com_2HS_DmgFr", // 200 0x63f600
		"_Com_2HS_DmgFrMult", // 201 0x63f60f
		"_Com_2HS_Dodge", // 202 0x63f61e
		"_Com_2HS_Equip", // 203 0x63f62d
		"_Com_2HS_Idle", // 204 0x63f63c
		"_Com_2HS_Knock_ProneB", // 205 0x63f64b
		"_Com_2HS_Knock_ProneF", // 206 0x63f65a
		"_Com_2HS_Limp", // 207 0x63f669
		"_Com_2HS_Limp_DmgBk", // 208 0x63f678
		"_Com_2HS_Limp_DmgFr", // 209 0x63f687
		"_Com_2HS_Maintenance", // 210 0x63f696
		"_Com_2HS_Gun_NoRecoil", // 211 0x63f6a5
		"_Com_2HS_Run", // 212 0x63f6b4
		"_Com_2HS_Run_DmgBk", // 213 0x63f6c3
		"_Com_2HS_Run_DmgFr", // 214 0x63f6d2
		"_Com_2HS_Gun_Shoot", // 215 0x63f6e1
		"_Com_2HS_Stunned", // 216 0x63f6f0
		"_Com_2HS_to_Default", // 217 0x63f6ff
		"_Com_2HS_to_Limp", // 218 0x63f70e
		"_Com_2HS_to_Run", // 219 0x63f71d
		"_Com_2HS_to_Walk", // 220 0x63f72c
		"_Com_2HS_Unequip", // 221 0x63f73b
		"_Com_2HS_Walk", // 222 0x63f74a
		"_Com_2HS_Walk_DmgBk", // 223 0x63f759
		"_Com_2HS_Walk_DmgFr", // 224 0x63f768
		"_Com_2HS_Laydown_ProneB", // 225 0x63f777
		"_Com_2HS_Laydown_ProneF", // 226 0x63f786
		"_Com_Una_Attack_Special1", // 227 0x63f795
		"_Com_Una_Attack_Special2", // 228 0x63f7a4
		"_Com_Una_Attack1", // 229 0x63f7b3
		"_Com_Una_Attack2", // 230 0x63f7c2
		"_Com_Una_Attack3", // 231 0x63f7d1
		"_Com_Una_Celebrate", // 232 0x63f7e0
		"_Com_Una_Death_Bio", // 233 0x63f7ef
		"_Com_Una_Death_Burst", // 234 0x63f7fe
		"_Com_Una_Death_ChestHole", // 235 0x63f80d
		"_Com_Una_Death_Electrical", // 236 0x63f81c
		"_Com_Una_Death_Fire", // 237 0x63f82b
		"_Com_Una_Death_Gib", // 238 0x63f83a
		"_Com_Una_Death_HeadShot", // 239 0x63f849
		"_Com_Una_Death_Lazer", // 240 0x63f858
		"_Com_Una_Death_Melting", // 241 0x63f867
		"_Com_Una_Disarm", // 242 0x63f876
		"_Com_Una_DmgBk", // 243 0x63f885
		"_Com_Una_DmgBkMult", // 244 0x63f894
		"_Com_Una_DmgFr", // 245 0x63f8a3
		"_Com_Una_DmgFrMult", // 246 0x63f8b2
		"_Com_Una_Dodge", // 247 0x63f8c1
		"_Com_Una_Elbow_Cross", // 248 0x63f8d0
		"_Com_Una_Elbow_Smash", // 249 0x63f8df
		"_Com_Una_Equip", // 250 0x63f8ee
		"_Com_Una_Armor_Equip", // 251 0x63f8fd
		"_Com_Una_Helmet_Equip", // 252 0x63f90c
		"_Com_Una_Idle", // 253 0x63f91b
		"_Com_Una_Idle_Stance_BOS", // 254 0x63f92a
		"_Com_Una_Idle_Stance_Hangdog", // 255 0x63f939
		"_Com_Una_Idle_Stance_NCR", // 256 0x63f948
		"_Com_Una_Idle_Stance_Powpapa", // 257 0x63f957
		"_Com_Una_Idle_Stance_Viper", // 258 0x63f966
		"_Com_Una_Kick_Axe", // 259 0x63f975
		"_Com_Una_Kick_Back", // 260 0x63f984
		"_Com_Una_Kick_Hook", // 261 0x63f993
		"_Com_Una_Kick_Roundhouse", // 262 0x63f9a2
		"_Com_Una_Kick_Scissor", // 263 0x63f9b1
		"_Com_Una_Kick_Side", // 264 0x63f9c0
		"_Com_Una_Kick_Snap", // 265 0x63f9cf
		"_Com_Una_Kick_Split", // 266 0x63f9de
		"_Com_Una_Kick_StepSide", // 267 0x63f9ed
		"_Com_Una_Kick_Sweep", // 268 0x63f9fc
		"_Com_Una_Knock_ProneB", // 269 0x63fa0b
		"_Com_Una_Knock_ProneF", // 270 0x63fa1a
		"_Com_Una_Laydown_ProneB", // 271 0x63fa29
		"_Com_Una_Laydown_ProneF", // 272 0x63fa38
		"_Com_Una_Limp", // 273 0x63fa47
		"_Com_Una_Limp_DmgBk", // 274 0x63fa56
		"_Com_Una_Limp_DmgFr", // 275 0x63fa65
		"_Com_Una_Punch_Backhand", // 276 0x63fa74
		"_Com_Una_Punch_Cross", // 277 0x63fa83
		"_Com_Una_Punch_Jab", // 278 0x63fa92
		"_Com_Una_Punch_Jab_Backhand", // 279 0x63faa1
		"_Com_Una_Punch_Jab_BackRidge", // 280 0x63fab0
		"_Com_Una_Punch_Ridgehand", // 281 0x63fabf
		"_Com_Una_Punch_Spearhand", // 282 0x63face
		"_Com_Una_Punch_Sucker", // 283 0x63fadd
		"_Com_Una_Punch_SuckerSpear", // 284 0x63faec
		"_Com_Una_PuncH_Uppercut", // 285 0x63fafb
		"_Com_Una_Run", // 286 0x63fb0a
		"_Com_Una_Run_DmgBk", // 287 0x63fb19
		"_Com_Una_Run_DmgFr", // 288 0x63fb28
		"_Com_Una_Shoot_Breath", // 289 0x63fb37
		"_Com_Una_Stunned", // 290 0x63fb46
		"_Com_Una_to_ComFly", // 291 0x63fb55
		"_Com_Una_to_Def", // 292 0x63fb64
		"_Com_Una_to_Limp", // 293 0x63fb73
		"_Com_Una_to_Run", // 294 0x63fb82
		"_Com_Una_to_Walk", // 295 0x63fb91
		"_Com_Una_Unequip", // 296 0x63fba0
		"_Com_Una_Armor_Unequip", // 297 0x63fbaf
		"_Com_Una_Helmet_Unequip", // 298 0x63fbbe
		"_Com_Una_Walk", // 299 0x63fbcd
		"_Com_Una_Walk_DmgBk", // 300 0x63fbdc
		"_Com_Una_Walk_DmgFr", // 301 0x63fbeb
		"_Com_Una_Pickup", // 302 0x63fbfa
		"_Com_Una_Burrowup", // 303 0x63fc09
		"_Com_Una_Burrowdown", // 304 0x63fc18
		"_ComFly_1H_Attack1", // 305 0x63fc27
		"_ComFly_1H_Attack2", // 306 0x63fc36
		"_ComFly_1H_Gun_Burst", // 307 0x63fc45
		"_ComFly_1H_Celebrate", // 308 0x63fc54
		"_ComFly_1H_DmgBk", // 309 0x63fc63
		"_ComFly_1H_DmgBkMult", // 310 0x63fc72
		"_ComFly_1H_DmgFr", // 311 0x63fc81
		"_ComFly_1H_DmgFrMult", // 312 0x63fc90
		"_ComFly_1H_Dodge", // 313 0x63fc9f
		"_ComFly_1H_Equip", // 314 0x63fcae
		"_ComFly_1H_Fly", // 315 0x63fcbd
		"_ComFly_1H_Fly_DmgBk", // 316 0x63fccc
		"_ComFly_1H_Fly_DmgFr", // 317 0x63fcdb
		"_ComFly_1H_Idle", // 318 0x63fcea
		"_ComFly_1H_Knock_ProneB", // 319 0x63fcf9
		"_ComFly_1H_Knock_ProneF", // 320 0x63fd08
		"_ComFly_1H_Maintenance", // 321 0x63fd17
		"_ComFly_1H_NoRecoil", // 322 0x63fd26
		"_ComFly_1H_Shoot", // 323 0x63fd35
		"_ComFly_1H_Stunned", // 324 0x63fd44
		"_ComFly_1H_Sweep", // 325 0x63fd53
		"_ComFly_1H_Throw", // 326 0x63fd62
		"_ComFly_1H_to_Com", // 327 0x63fd71
		"_ComFly_1H_to_DefFly", // 328 0x63fd80
		"_ComFly_1H_to_Fly", // 329 0x63fd8f
		"_ComFly_1H_Unequip", // 330 0x63fd9e
		"_ComFly_Una_Attack_Special1", // 331 0x63fdad
		"_ComFly_Una_Attack_Special2", // 332 0x63fdbc
		"_ComFly_Una_Attack1", // 333 0x63fdcb
		"_ComFly_Una_Attack2", // 334 0x63fdda
		"_ComFly_Una_Celebrate", // 335 0x63fde9
		"_ComFly_Una_DmgBk", // 336 0x63fdf8
		"_ComFly_Una_DmgBkMult", // 337 0x63fe07
		"_ComFly_Una_DmgFr", // 338 0x63fe16
		"_ComFly_Una_DmgFrMult", // 339 0x63fe25
		"_ComFly_Una_Dodge", // 340 0x63fe34
		"_ComFly_Una_Equip", // 341 0x63fe43
		"_ComFly_Una_Fly", // 342 0x63fe52
		"_ComFly_Una_Idle", // 343 0x63fe61
		"_ComFly_Una_Knock_ProneB", // 344 0x63fe70
		"_ComFly_Una_Knock_ProneF", // 345 0x63fe7f
		"_ComFly_Una_Laydown_ProneB", // 346 0x63fe8e
		"_ComFly_Una_Laydown_ProneF", // 347 0x63fe9d
		"_ComFly_Una_Stunned", // 348 0x63feac
		"_ComFly_Una_To_Com", // 349 0x63febb
		"_ComFly_Una_To_DefFly", // 350 0x63feca
		"_ComFly_Una_To_Fly", // 351 0x63fed9
		"_ComFly_Una_Unequip", // 352 0x63fee8
		"_Crouch_Una_DmgFr", // 353 0x63fef7
		"_Crouch_Una_DmgFrMult", // 354 0x63ff06
		"_Crouch_Una_Idle", // 355 0x63ff15
		"_Crouch_Una_Knock_ProneB", // 356 0x63ff24
		"_Crouch_Una_Knock_ProneF", // 357 0x63ff33
		"_Crouch_Una_to_Default", // 358 0x63ff42
		"_Crouch_Una_Use", // 359 0x63ff51
		"_Def_1H_Fidget", // 360 0x63ff60
		"_Def_1H_Idle", // 361 0x63ff6f
		"_Def_1H_Limp", // 362 0x63ff7e
		"_Def_1H_Run", // 363 0x63ff8d
		"_Def_1H_Walk", // 364 0x63ff9c
		"_Def_1H_to_Com", // 365 0x63ffab
		"_Def_1H_to_DefFly", // 366 0x63ffba
		"_Def_1H_to_Limp", // 367 0x63ffc9
		"_Def_1H_to_Run", // 368 0x63ffd8
		"_Def_1H_to_Walk", // 369 0x63ffe7
		"_Def_2HC_Fidget", // 370 0x63fff6
		"_Def_2HC_Idle", // 371 0x640005
		"_Def_2HC_to_Com", // 372 0x640014
		"_Def_2HC_to_Limp", // 373 0x640023
		"_Def_2HC_to_Run", // 374 0x640032
		"_Def_2HC_to_Walk", // 375 0x640041
		"_Def_2HH_Fidget", // 376 0x640050
		"_Def_2HH_Idle", // 377 0x64005f
		"_Def_2HH_to_Com", // 378 0x64006e
		"_Def_2HH_to_Limp", // 379 0x64007d
		"_Def_2HH_to_Run", // 380 0x64008c
		"_Def_2HH_to_Walk", // 381 0x64009b
		"_Def_2HO_Fidget", // 382 0x6400aa
		"_Def_2HO_Idle", // 383 0x6400b9
		"_Def_2HO_to_Com", // 384 0x6400c8
		"_Def_2HO_to_Limp", // 385 0x6400d7
		"_Def_2HO_to_Run", // 386 0x6400e6
		"_Def_2HO_to_Walk", // 387 0x6400f5
		"_Def_2HR_Fidget", // 388 0x640104
		"_Def_2HR_Idle", // 389 0x640113
		"_Def_2HR_to_Com", // 390 0x640122
		"_Def_2HR_to_Limp", // 391 0x640131
		"_Def_2HR_to_Run", // 392 0x640140
		"_Def_2HR_to_Walk", // 393 0x64014f
		"_Def_2HS_Fidget", // 394 0x64015e
		"_Def_2HS_Idle", // 395 0x64016d
		"_Def_2HS_to_Com", // 396 0x64017c
		"_Def_2HS_to_Limp", // 397 0x64018b
		"_Def_2HS_to_Run", // 398 0x64019a
		"_Def_2HS_to_Walk", // 399 0x6401a9
		"_Def_Una_Fidget_HighSkill", // 400 0x6401b8
		"_Def_Una_Fidget_LowSkill", // 401 0x6401c7
		"_Def_Una_Fidget_MedSkill", // 402 0x6401d6
		"_Def_Una_Gesture_Beckon", // 403 0x6401e5
		"_Def_Una_Gesture_Consume", // 404 0x6401f4
		"_Def_Una_Gesture_Dismiss", // 405 0x640203
		"_Def_Una_Gesture_Drink", // 406 0x640212
		"_Def_Una_Gesture_Inject", // 407 0x640221
		"_Def_Una_Gesture_LOL", // 408 0x640230
		"_Def_Una_Gesture_No", // 409 0x64023f
		"_Def_Una_Gesture_Point", // 410 0x64024e
		"_Def_Una_Gesture_Salute", // 411 0x64025d
		"_Def_Una_Gesture_Talk", // 412 0x64026c
		"_Def_Una_Gesture_Vomit", // 413 0x64027b
		"_Def_Una_Gesture_Wave", // 414 0x64028a
		"_Def_Una_Gesture_Yes", // 415 0x640299
		"_Def_Una_Idle", // 416 0x6402a8
		"_Def_Una_PanicRun", // 417 0x6402b7
		"_Def_Una_PipBoy_Use_End", // 418 0x6402c6
		"_Def_Una_PipBoy_Use_Idle", // 419 0x6402d5
		"_Def_Una_PipBoy_Use_Start", // 420 0x6402e4
		"_Def_Una_to_Climb_Down", // 421 0x6402f3
		"_Def_Una_to_Climb_Up", // 422 0x640302
		"_Def_Una_to_Com", // 423 0x640311
		"_Def_Una_to_Crouch", // 424 0x640320
		"_Def_Una_to_DefFly", // 425 0x64032f
		"_Def_Una_to_Limp", // 426 0x64033e
		"_Def_Una_to_Sit_Left", // 427 0x64034d
		"_Def_Una_to_Sit_Right", // 428 0x64035c
		"_Def_Una_to_Wade", // 429 0x64036b
		"_Def_Una_to_Walk", // 430 0x64037a
		"_Def_Una_Use", // 431 0x640389
		"_Def_Una_Wade", // 432 0x640398
		"_Def_Una_Wade_Fidget", // 433 0x6403a7
		"_Def_Una_Wade_Idle", // 434 0x6403b6
		"_Def_Una_Pickup", // 435 0x6403c5
		"_DefFly_1H_Fidget", // 436 0x6403d4
		"_DefFly_1H_Fly", // 437 0x6403e3
		"_DefFly_1H_Idle", // 438 0x6403f2
		"_DefFly_1H_to_ComFly", // 439 0x640401
		"_DefFly_1H_to_Def", // 440 0x640410
		"_DefFly_1H_to_Fly", // 441 0x64041f
		"_DefFly_1H_Use", // 442 0x64042e
		"_DefFly_Una_Fidget", // 443 0x64043d
		"_DefFly_Una_Fly", // 444 0x64044c
		"_DefFly_Una_Idle", // 445 0x64045b
		"_DefFly_Una_to_ComFly", // 446 0x64046a
		"_DefFly_Una_to_Def", // 447 0x640479
		"_DefFly_Una_to_Fly", // 448 0x640488
		"_DefFly_Una_Use", // 449 0x640497
		"_Head_0_Idle", // 450 0x6404a6
		"_Head_0_to_15_Turn", // 451 0x6404b5
		"_Head_0_to_-15_Turn", // 452 0x6404c4
		"_Head_15_Idle", // 453 0x6404d3
		"_Head_-15_Idle", // 454 0x6404e2
		"_Head_15_to_30_Turn", // 455 0x6404f1
		"_Head_-15_to_-30_Turn", // 456 0x640500
		"_Head_30_Idle", // 457 0x64050f
		"_Head_-30_Idle", // 458 0x64051e
		"_Head_30_to_45_Turn", // 459 0x64052d
		"_Head_-30_to_-45_Turn", // 460 0x64053c
		"_Head_45_Idle", // 461 0x64054b
		"_Head_-45_Idle", // 462 0x64055a
		"_Head_45_to_60_Turn", // 463 0x640569
		"_Head_-45_to_-60_Turn", // 464 0x640578
		"_Head_60_Idle", // 465 0x640587
		"_Head_-60_Idle", // 466 0x640596
		"_Head_60_to_75_Turn", // 467 0x6405a5
		"_Head_-60_to_-75_Turn", // 468 0x6405b4
		"_Head_75_Idle", // 469 0x6405c3
		"_Head_-75_Idle", // 470 0x6405d2
		"_Head_75_to_90_Turn", // 471 0x6405e1
		"_Head_-75_to_-90_Turn", // 472 0x6405f0
		"_Head_90_Idle", // 473 0x6405ff
		"_Head_-90_Idle", // 474 0x64060e
		"_ProneB_1H_to_Com", // 475 0x64061d
		"_ProneB_1H_to_ComFly", // 476 0x64062c
		"_ProneB_2HC_to_Com", // 477 0x64063b
		"_ProneB_2HG_to_Com", // 478 0x64064a
		"_ProneB_2HH_to_Com", // 479 0x640659
		"_ProneB_2HO_to_Com", // 480 0x640668
		"_ProneB_2HR_to_Com", // 481 0x640677
		"_ProneB_2HS_to_Com", // 482 0x640686
		"_ProneB_2W_to_Com", // 483 0x640695
		"_ProneB_Una_Death", // 484 0x6406a4
		"_ProneB_Una_DmgFr", // 485 0x6406b3
		"_ProneB_Una_DmgFrMult", // 486 0x6406c2
		"_ProneB_Una_Idle", // 487 0x6406d1
		"_ProneB_Una_to_Com", // 488 0x6406e0
		"_ProneB_Una_to_ComFly", // 489 0x6406ef
		"_ProneB_Una_Twitch", // 490 0x6406fe
		"_ProneF_1H_to_Com", // 491 0x64070d
		"_ProneF_2HC_to_Com", // 492 0x64071c
		"_ProneF_2HG_to_Com", // 493 0x64072b
		"_ProneF_2HH_to_Com", // 494 0x64073a
		"_ProneF_2HO_to_Com", // 495 0x640749
		"_ProneF_2HR_to_Com", // 496 0x640758
		"_ProneF_2HS_to_Com", // 497 0x640767
		"_ProneF_2W_to_Com", // 498 0x640776
		"_ProneF_Una_Death", // 499 0x640785
		"_ProneF_Una_DmgBk", // 500 0x640794
		"_ProneF_Una_DmgBkMult", // 501 0x6407a3
		"_ProneF_Una_Idle", // 502 0x6407b2
		"_ProneF_Una_to_Com", // 503 0x6407c1
		"_ProneF_Una_Twitch", // 504 0x6407d0
		"_Sit_Una_DmgFr", // 505 0x6407df
		"_Sit_Una_DmgFrMult", // 506 0x6407ee
		"_Sit_Una_Gesture_Consume", // 507 0x6407fd
		"_Sit_Una_Gesture_Drink", // 508 0x64080c
		"_Sit_Una_Gesture_Fidget", // 509 0x64081b
		"_Sit_Una_Gesture_Talk", // 510 0x64082a
		"_Sit_Una_Idle", // 511 0x640839
		"_Sit_Una_to_Def_Left", // 512 0x640848
		"_Sit_Una_to_Def_Right", // 513 0x640857
		"_Open_In", // 514 0x640866
		"_Open_Out", // 515 0x640875
	};

}

namespace F3
{
	GameAnimationCache::GameAnimationCache()
	{
	}

	GameAnimationCache::~GameAnimationCache()
	{
	}

	DWORD GameAnimationCache::GetGroupCount()
	{
		return 516;
	}

	const char* GameAnimationCache::GetGroupSuffix(DWORD group)
	{
		return group < GetGroupCount() ? g_animationGroups[group].c_str() : NULL;
	}

	void GameAnimationCache::ReadAnimations(Gfx_AnimationObject* object, std::string name,
		BOOL requested)
	{
		std::string resource;
		DWORD first;
		DWORD end;
		if (static_cast<BYTE>(requested))
		{
			first = 416;
			end = 417;
		}
		else
		{
			first = 0;
			end = 516;
		}
		for (DWORD group = first; group < end; ++group)
		{
			LONG number = 1;
			for (;;)
			{
				std::string ordinal("_");
				// The original _itoa local's source bound is not yet established.
				// Its observed stack slot is smaller than this retained host bound.
				char digits[32];
				_itoa(number, digits, 10);
				if (static_cast<unsigned int>(number) < 10) ordinal += "0";
				ordinal += digits;
				resource = name + g_animationGroups[group] + ordinal + ".gr2";
				DWORD size = 0;
				BYTE* bytes = ReadSkeletonResource(resource, &size);
				if (bytes == NULL)
				{
					//45D57C..45D5B9: original unchecked30-byte prefix buffer.
					char prefix[30];
					memset(prefix, 0, sizeof(prefix));
					strncpy(prefix, name.c_str(), name.length() - 3);
					resource = std::string(prefix) + g_animationGroups[group] + ordinal + ".gr2";
					bytes = ReadSkeletonResource(resource, &size);
					if (bytes == NULL) break;
				}
				JE::g_GrannyError = 0;
				if (!object->LoadAnimationFileFromBuffer(group, size, bytes, 10.0f))
				{
					JE::FatalError("GameAnimationCache::ReadAnimations() - Unable to load data from animation file buffer.");
				}
				if (JE::g_GrannyError)
				{
					JE::LogDebugString("GameAnimationCache::ReadAnimations() - LoadAnimationFileFromBuffer (%s) Granny error\n",
						resource.c_str());
				}
				::operator delete(bytes);
				size = 0;
				resource = name + g_animationGroups[group] + ordinal + ".ini";
				JE::Configuration::Settings settings;
				BYTE loaded = JE::Configuration::ReadResourceConfigurationFile(resource, settings);
				if (!loaded)
				{
					char prefix[30];
					memset(prefix, 0, sizeof(prefix));
					strncpy(prefix, name.c_str(), name.length() - 3);
					resource = prefix + g_animationGroups[group] + ordinal + ".ini";
					loaded = JE::Configuration::ReadResourceConfigurationFile(resource, settings);
				}
				if (loaded)
				{
					object->ClearAnimationCues(group, static_cast<DWORD>(number - 1));
					JE::Configuration::Settings::iterator section = settings.find("ANIM_CUES");
					if (section != settings.end())
					{
						JE::Configuration::Values& values = section->second;
						LONG cueCount = 0;
						JE::Configuration::Values::iterator count = values.find("count");
						if (count != values.end()) cueCount = atol(count->second.c_str());
						if (cueCount > 0)
						{
							LONG index = 1;
							do
							{
								{
									std::string key;
									std::string cue;
									JE::FormatString(key, "cue%d", index);
									JE::Configuration::Values::iterator entry = values.find(key);
									if (entry != values.end())
									{
										cue = entry->second.c_str();
										JE::FormatString(key, "time%d", index);
										entry = values.find(key);
										if (entry != values.end())
										{
											const DWORD milliseconds = static_cast<DWORD>(atol(entry->second.c_str()));
											object->AddAnimationCue(group, static_cast<DWORD>(number - 1),
												milliseconds * 0.001f, cue.c_str());
										}
									}
								}
								++index;
							}
							while (--cueCount);
						}
					}
				}
				//45DECD advances before the local map and ordinal are destroyed.
				++number;
			}
		}
	}
}
