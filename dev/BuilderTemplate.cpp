/**
 * @file:	BuilderTemplate.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/29 16:14:29 Friday
 * @brief:
 **/

#include <algorithm>
#include <iostream>

// ================================================================================================
// Lib

template <typename SpiBaseType, typename BuilderPatternReturnType>
struct TemplatedApp
{
public:
    // Default constructor
    TemplatedApp()
        : m_spi_ptr(nullptr) {}

    // disallow copying, only move
    // TemplatedApp(const TemplatedApp& other) = delete;

    template <typename SPI>
    BuilderPatternReturnType registerApp(SPI& spi)
    {
        this->m_spi_ptr = &spi;
        return std::move(static_cast<BuilderPatternReturnType&&>(*this));
    }

    SpiBaseType* m_spi_ptr = nullptr;
};

// Define the App class as a template
template <typename T>
class App : public TemplatedApp<T, App<T>>
{
public:
    void run()
    {
        std::cout << "App is running." << std::endl;
        // Additional logic for running the app can be added here
    }

    void send(std::string msg) {}
};

// ================================================================================================
// Biz

struct NewOrderSingle
{
    const std::string toStr() const
    {
        return "";
    }
};

class SpiServer
{
public:
    void init(App<SpiServer>* api)
    {
        this->m_api = api;
    }

    void onMessage(const NewOrderSingle& order)
    {
        this->m_api->send(order.toStr());
    }

protected:
    App<SpiServer>* m_api;
};

// ================================================================================================
// Main
// ================================================================================================

int main(int argc, char** argv)
{
    // biz
    auto spiServer = SpiServer();

    // lib
    auto app = App<SpiServer>();
    app.registerApp(spiServer);

    // biz
    spiServer.init(&app);

    // lib run
    app.run();

    // biz
    auto msg = NewOrderSingle();
    spiServer.onMessage(msg);

    return 0;
}
