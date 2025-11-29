#include "Constellation.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void Constellation::WriteHooks()
	{
		LoadSkydomePatch1();
		EnterConstellationPatch1();
		EnterConstellationPatch2();
		ExitConstellationPatch1();
		ExitConstellationPatch2();
		KinectPatch();
		UpdateConstellationPatch();
	}

	void Constellation::LoadSkydomePatch1()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::LoadSkydome, 0x159);
		REL::make_pattern<"48 89 8C DF 98 00 00 00">().match_or_fail(hook.address());

		auto SetCImageShader = +[](RE::BSShaderProperty* shader, std::uint32_t index)
		{
			auto& controller = CustomSkillsManager::_cImageControllers[index];
			controller.SetShader(shader);
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x28)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(edx, esi);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				nop(0x16);
				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(SetCImageShader) };

		REL::safe_fill(hook.address(), REL::NOP, 0x28);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::EnterConstellationPatch1()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Rotate, 0x3A4);

		auto EnterTree = +[](std::uint32_t a_index)
		{
			auto& controller = CustomSkillsManager::_cImageControllers[a_index];
			controller.Enter();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				mov(ecx, dword[rdi + offsetof(RE::StatsMenu::RUNTIME_DATA, selectedTree)]);
				call(ptr[rip + funcLbl]);
				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		auto patch = new Patch(reinterpret_cast<std::uintptr_t>(EnterTree), hook.address() + 0x14);
		patch->ready();

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x14);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void Constellation::EnterConstellationPatch2()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Animate, 0x149);

		auto EnterTree = +[](std::uint32_t a_index)
		{
			auto& controller = CustomSkillsManager::_cImageControllers[a_index];
			controller.Enter();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x22)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(ecx, dword[rbx + offsetof(RE::StatsMenu::RUNTIME_DATA, selectedTree)]);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(EnterTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x22);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::ExitConstellationPatch1()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::ProcessRotateEvent,
			0x141);

		auto ExitTree = +[](std::uint32_t a_index)
		{
			auto& controller = CustomSkillsManager::_cImageControllers[a_index];
			if (controller.shader && !*CustomSkillsManager::IsSingleSkillMode) {
				controller.Exit();
			}
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x62)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(ecx, dword[rdi + offsetof(RE::StatsMenu::RUNTIME_DATA, selectedTree)]);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				nop(0x4C, false);
				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(ExitTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x62);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::ExitConstellationPatch2()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::ProcessRotateEvent,
			0x2A7);

		auto ExitTree = +[](std::uint32_t a_index)
		{
			auto& controller = CustomSkillsManager::_cImageControllers[a_index];
			if (controller.shader) {
				controller.Exit();
			}
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x4D)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(ecx, dword[rdi + offsetof(RE::StatsMenu::RUNTIME_DATA, selectedTree)]);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				nop(0x37, false);
				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(ExitTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x4D);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::KinectPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::GotoNode, 0xAB);

		auto SetSelectedTree = +[](RE::StatsMenu* a_statsMenu, std::uint32_t a_newIndex)
		{
			const std::uint32_t oldIndex = a_statsMenu->GetRuntimeData().selectedTree;
			auto& controllers = CustomSkillsManager::_cImageControllers;

			controllers[oldIndex].Exit();

			a_statsMenu->GetRuntimeData().selectedTree = a_newIndex;
			controllers[a_newIndex].Enter();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x69)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(rcx, rdi);
				mov(edx, esi);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				nop(0x53, false);
				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(SetSelectedTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x69);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::UpdateConstellationPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::ProcessMessage, 0x1020);

		auto UpdateConstellation = +[](std::uint32_t a_index)
		{
			CustomSkillsManager::_cImageControllers[a_index].Update();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
				: Xbyak::CodeGenerator(0xAF)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				mov(ecx, edi);
				call(ptr[rip + funcLbl]);
				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		Patch
			patch{ reinterpret_cast<std::uintptr_t>(UpdateConstellation), hook.address() + 0xAF};
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0xAF);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}
}
