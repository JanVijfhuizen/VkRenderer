#include "pch.h"
#include "WindowHandler.h"

vi::WindowHandler::Info::Info() = default;

vi::WindowHandler::Info::Info(const Info& other)
{
	resolution = other.resolution;
	name = String(other.name, GMEM);
}

vi::WindowHandler::WindowHandler(const Info& info) : info(info)
{

}

const vi::WindowHandler::Info& vi::WindowHandler::GetInfo() const
{
	return info;
}
