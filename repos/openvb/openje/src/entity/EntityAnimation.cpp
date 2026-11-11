#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "World.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/AnimationInstance.hpp"

namespace JE
{
	namespace
	{
		DWORD WorldTime()
		{
			IWorld* world = g_World; //Original virtual-base conversion before slot0 dispatch.
			return world->GetTime();
		}
	}

	void Entity::ResolveAnimation(DWORD group, DWORD file, DWORD* outGroup, DWORD* outFile)
	{
		*outGroup = group;
		*outFile = file;
	}
	DWORD Entity::GetIdleAnimationGroup() const { return 0; }
	DWORD Entity::EndAnimationNow() { return animationEndTime = WorldTime(); }
	float Entity::GetAnimationDuration() const
	{
		float duration = 0.0f;
		if (model)
		{
			Gfx_AnimationInstance* animation = model->animation;
			duration = animation->animationObject->GetDuration(currentAnimationGroup, currentAnimationIndex, 0);
		}
		return duration;
	}
	BYTE Entity::IsDead() const { return state == 7; }
	void Entity::OnAnimationChanged(DWORD, DWORD) {} // Original shared408090, including Creature40D440 thunk.
	DWORD Entity::GetAnimationTime() const
	{
		if (frozenAnimation) return frozenAnimationTime;
		return WorldTime() - animationStartTime;
	}

	void Entity::PlayAnimation(DWORD group, DWORD file, WORD loops, BYTE reverse, DWORD timeOffset)
	{
		if (!animationEnabled || !model || !model->animation) return;
		Gfx_AnimationInstance* animation = model->animation; // Captured before virtual resolver548DA1.
		DWORD requestedGroup = group;
		ResolveAnimation(group, file, &group, &file);
		if (!group) return;
		if (requestedGroup != group) reverse = 0;
		currentAnimationGroup = group;
		currentAnimationIndex = file;
		animationReverse = reverse;
		animationStartTime = WorldTime() - timeOffset;
		animation->SelectAnimation(group, file, 0, 0.1f, 1, loops);
		animation->SetSpeed(1.0f);
		animation->reverse = reverse;
		const DWORD negativeDuration = DWORD(static_cast<__int64>(
			double(animation->animationObject->GetDuration(group, file, 0)) * -1000.0));
		if (loops <= 1)
		{
			animationEndTime = animationStartTime - negativeDuration;
			animationLooping = loops == 0;
		}
		else animationEndTime = animationStartTime - loops * negativeDuration;
		if (GetEntityEnvironment().NetworkMode() & 1)
			OnAnimationChanged(currentAnimationGroup, currentAnimationIndex);
	}

	void Entity::QueueAnimation(DWORD group, DWORD file, DWORD loops, BYTE reverse,
		DWORD timeOffset, BYTE clearPending)
	{
		EntityAnimationRequest request;
		request.group = group;
		request.file = file;
		request.loops = loops;
		request.reverse = reverse;
		request.timeOffset = timeOffset;
		request.clearPending = clearPending;
		animationQueue.push_back(request);
	}

	LONG Entity::QueueAnimationPercent(DWORD group, DWORD file, DWORD loops, DWORD reverse,
		float percent, DWORD clearPending)
	{
		EntityAnimationRequest request;
		request.group = group;
		request.file = file;
		request.loops = loops;
		request.reverse = BYTE(reverse);
		request.clearPending = BYTE(clearPending);
		ResolveAnimation(group, file, &clearPending, &reverse);
		LONG result = 0;
		float duration = loops > 0 ? 0.0f : 1.0f;
		if (model && model->animation)
		{
			Gfx_AnimationInstance* animation = model->animation;
			duration = animation->animationObject->GetDuration(clearPending, reverse, 0);
		}
		if (loops > 0) result = model ? LONG(static_cast<__int64>(1000.0f * duration)) : 2000;
		duration *= percent;
		request.timeOffset = DWORD(static_cast<__int64>(duration * 10.0f));
		animationQueue.push_back(request);
		return result;
	}

	void Entity::FreezeAnimation()
	{
		if (!frozenAnimation)
		{
			frozenAnimation = 1;
			frozenAnimationTime = WorldTime();
		}
	}
	void Entity::UnfreezeAnimation()
	{
		if (frozenAnimation)
		{
			frozenAnimation = 0;
			DWORD elapsed = WorldTime() - frozenAnimationTime;
			animationStartTime += elapsed;
			animationEndTime += elapsed;
		}
	}
	void Entity::UpdateAnimationQueue()
	{
		// Actual animation suffix of54C570. Effect events, messages, scripts and
		// action-budget dispatch before54C891 are not replaced by this factoring.
		if (animationLooping)
		{
			if (animationQueue.empty()) return;
		}
		else
		{
			if (WorldTime() < animationEndTime) return;
			if (animationStartTime && animationStartTime < animationEndTime && model)
				model->ProcessAnimationCues(animationEndTime - animationStartTime);
			if (animationQueue.empty())
			{
				if (!suppressIdleAnimation && !IsDead())
				{
					DWORD group = GetIdleAnimationGroup();
					if (group) PlayAnimation(group, 0, 0, 0, 0);
				}
				return;
			}
		}
		EntityAnimationRequest request = animationQueue.front();
		animationQueue.pop_front();
		PlayAnimation(request.group, request.file, WORD(request.loops), request.reverse, request.timeOffset);
		if (request.clearPending) pendingAnimation = 0;
	}
}
