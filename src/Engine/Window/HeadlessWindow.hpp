#pragma once
#include "IWindow.hpp"
#include "pch.hpp"

enum class HeadlessEventType {
	AVAILABLE,
	PLAY,
	SAVE,
    MARK
};

HeadlessEventType StringToEventType(const std::string& eventStr);

struct HeadlessEvent {
    long long ts; // Timestamp in microseconds
    HeadlessEventType type; // The type of the event
    std::vector<std::string> params; // Event parameters

    HeadlessEvent(long long ts, HeadlessEventType type, std::vector<std::string> params)
        : ts(ts)
        , type(type)
        , params(std::move(params)) {};
    bool operator <(const HeadlessEvent& other) const
	{
		return ts < other.ts;
	}
};

class HeadlessWindow : public IWindow {
public:
    HeadlessWindow(std::string eventsPath)
        : m_eventsPath(eventsPath)
    {
                ParseEvents();
    };
    virtual void Create(const std::string& title, int width, int height, EngineCore::IApp& app) override;
    virtual void Destroy() override;
    virtual void Update() override;
    virtual bool ShouldClose() override;
    virtual void GetWindowSize(int& width, int& height) const override;
    virtual std::optional<VkSurfaceKHR> CreateSurface(VkInstance instance) override;
    bool IsHeadless() const override;

private:
    void ParseEvents();
    std::vector<HeadlessEvent> events;


void ProcessEvent(const HeadlessEvent& param1);

private:
    std::string m_eventsPath;
	bool m_shouldClose = false;
	int m_width = 0;
	int m_height = 0;
	EngineCore::IApp* m_app = nullptr;

private:
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_lastUpdateTime;
};
