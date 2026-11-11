#pragma once

#include <d3dx8.h>

class OCTREE_Camera;
namespace JE
{
	namespace VfxFrame
	{
		double RandomRange(float first,float last); //4B8310, descriptive name/return spelling.
		void SetCamera(OCTREE_Camera* camera); //4B81C0,707F0C.
		OCTREE_Camera* GetCamera(); //4B81D0.
		void SetWind(float x,float y,float z); //4B81E0, three native values.
		void GetWind(float& x,float& y,float& z); //4B8200, three native outputs.
		void AddWind(float x,float y,float z); //4B8230, three native values.
		void SetWind(const D3DXVECTOR3& wind); //Vector convenience.
		void GetWind(D3DXVECTOR3& wind); //Vector convenience.
		void AddWind(const D3DXVECTOR3& wind); //Vector convenience.
		void AdvanceWind(float elapsedSeconds); //4B8270, ignored native time argument; publish and clear wind.
	}
}
