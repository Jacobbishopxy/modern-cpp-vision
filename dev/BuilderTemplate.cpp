/**
 * @file:	BuilderTemplate.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/29 16:14:29 Friday
 * @brief:
 **/

#include <format>
#include <iostream>

// ================================================================================================
// Lib

template <typename SpiBaseType, typename BuilderPatternReturnType>
struct TemplatedApp
{
public:
    // Default constructor
    TemplatedApp() = default;

    // Move constructor and assignment operator
    TemplatedApp(TemplatedApp&&) noexcept = default;
    TemplatedApp& operator=(TemplatedApp&&) noexcept = default;

    // Disallow copying
    TemplatedApp(const TemplatedApp&) = delete;
    TemplatedApp& operator=(const TemplatedApp&) = delete;

    // Register App with a SpiBaseType instance
    BuilderPatternReturnType registerApp(SpiBaseType& spi)
    {
        m_spi_ptr = &spi;
        return static_cast<BuilderPatternReturnType&&>(*this);  // Use static_cast for clarity
    }

    // Provide a getter for the SPI pointer
    [[nodiscard]] SpiBaseType* getSpiPtr() const noexcept
    {
        return m_spi_ptr;
    }

private:
    SpiBaseType* m_spi_ptr = nullptr;  // Pointer to SPI interface
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

    void send(std::string msg)
    {
        std::cout << "Sending message: " << msg << std::endl;
    }
};

// ================================================================================================
// Biz

struct NewOrderSingle
{
    std::string Symbol;
    uint Side;
    float Price;
    uint Volume;

    const std::string toStr() const
    {
        return std::format("{}_{}_{}_{}", this->Symbol, this->Side, this->Price, this->Volume);
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
    auto msg = NewOrderSingle{.Symbol = "000001", .Side = 2, .Price = 11.1, .Volume = 2000};
    spiServer.onMessage(msg);

    return 0;
}
