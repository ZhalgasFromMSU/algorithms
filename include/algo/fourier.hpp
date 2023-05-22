#include <vector>

namespace NAlgo {

    class TTransformer {
    public:
        TTransformer(int base) noexcept;

        // Fast-Fourier transformation
        std::vector<int> Transform(const std::vector<int>& coeffs) const noexcept;

        // Reverse FFT
        std::vector<int> ReverseTransform(const std::vector<int>& values) const noexcept;

    private:
        int Primitive_;
    };

} // NAlgo
