/**
 * @file:	BuilderTemplate.cpp
 * @author:	Jacob Xie
 * @date:	2024/11/29 16:14:29 Friday
 * @brief:
 **/

#include <fmt/format.h>

#include <iostream>

// ================================================================================================
// Lib

// Adt
struct NewOrderSingle
{
    std::string Symbol;
    uint Side;
    float Price;
    uint Volume;

    const std::string toStr() const
    {
        return fmt::format("{}_{}_{}_{}", this->Symbol, this->Side, this->Price, this->Volume);
    }
};

// ISpi interface
struct ISpi
{
    virtual void procNewOrder(const NewOrderSingle& order) = 0;
    virtual ~ISpi() = default;
};

// Template Constraint
template <typename T>
concept IsSpi = std::is_base_of_v<ISpi, T>;

// TemplatedApp, builder
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
template <IsSpi T>
class App : public TemplatedApp<T, App<T>>
{
public:
    void run()
    {
        std::cout << "App is running." << std::endl;
        // Additional logic for running the app can be added here
    }

    void receiveNewOrder(const NewOrderSingle& order)
    {
        if (auto* spi = this->getSpiPtr())
        {
            spi->procNewOrder(order);  // Ensure T's sendMessage is called
        }
        else
        {
            std::cerr << "SPI pointer is not set!" << std::endl;
        }
    }
};

// ================================================================================================
// Biz

// Example implementation of ISpi
class SpiMock : public ISpi
{
public:
    void procNewOrder(const NewOrderSingle& order) override
    {
        auto s = order.toStr();
        std::cout << "SpiMock.procNewOrder > " << s << std::endl;
    }
};

// ================================================================================================
// Main
// ================================================================================================

int main(int argc, char** argv)
{
    // biz
    SpiMock spi;
    // lib
    App<SpiMock> app;

    auto msg = NewOrderSingle{.Symbol = "000001", .Side = 2, .Price = 11.1, .Volume = 2000};

    // app run
    app.registerApp(spi).run();

    app.receiveNewOrder(msg);

    return 0;
}
