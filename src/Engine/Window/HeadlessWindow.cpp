#include "HeadlessWindow.hpp"

void HeadlessWindow::Create(const std::string& title, int width, int height, EngineCore::IApp& app)
{
    m_width = width;
    m_height = height;
    m_app = &app;

    m_startTime = std::chrono::high_resolution_clock::now();
    m_lastUpdateTime = m_startTime;
}
void HeadlessWindow::Destroy()
{
}

void HeadlessWindow::Update()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_startTime).count();

    while (!events.empty() && events.front().ts <= elapsedTime) {
        ProcessEvent(events.front());
        events.erase(events.begin());
    }

    m_lastUpdateTime = currentTime;
}

bool HeadlessWindow::ShouldClose()
{
    return events.empty();
}

void HeadlessWindow::GetWindowSize(int& width, int& height) const
{
    width = m_width;
    height = m_height;
}

std::optional<VkSurfaceKHR> HeadlessWindow::CreateSurface(VkInstance instance)
{
    return std::nullopt;
}

bool HeadlessWindow::IsHeadless() const
{
    return true;
}

void HeadlessWindow::ParseEvents()
{
    std::ifstream eventsFile(m_eventsPath);
    std::string line;

    while (getline(eventsFile, line)) {
        std::istringstream iss(line);
        long long ts;
        std::string eventStr;
        std::vector<std::string> params;

        iss >> ts >> eventStr;

        // Convert the event string to an event type
        HeadlessEventType eventType = StringToEventType(eventStr);

        // Read remaining parameters into params vector
        std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter(params));

        events.emplace_back(ts, eventType, params);
    }

    std::sort(events.begin(), events.end());
}

void HeadlessWindow::ProcessEvent(const HeadlessEvent& event)
{
    switch (event.type) {
    case HeadlessEventType::AVAILABLE: // Make a swap chain image available for rendering
        m_app->PresentImage();
        break;
    case HeadlessEventType::PLAY: // Set animation playback time and rate
    {
        // std::cout<< "PLAY " << event.params[0] << " " << event.params[1] << std::endl;
        float playbackTime = std::stof(event.params[0]);
        float playbackRate = std::stof(event.params[1]);
        m_app->SetPlaybackTimeAndRate(playbackTime, playbackRate);
        break;
    }
    case HeadlessEventType::SAVE: // Save the current frame as a PPM file
        // std::cout << "SAVE " << event.params[0] << std::endl;
        m_app->SaveFrame(event.params[0]);
        break;
    case HeadlessEventType::MARK: // Output a debug mark
        std::cout << "MARK ";
        for (auto& param : event.params) {
            std::cout << param << " ";
        }
        std::cout << std::endl;
        break;
    }
}

HeadlessEventType StringToEventType(const std::string& eventStr)
{
    if (eventStr == "AVAILABLE")
        return HeadlessEventType::AVAILABLE;
    if (eventStr == "PLAY")
        return HeadlessEventType::PLAY;
    if (eventStr == "SAVE")
        return HeadlessEventType::SAVE;
    if (eventStr == "MARK")
        return HeadlessEventType::MARK;
    // Add more event types as needed
    throw std::runtime_error("Unknown event type: " + eventStr);
}
