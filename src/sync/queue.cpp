module;

#include <atomic>
#include <vector>

export module algo:lfqueue;

export namespace algo {

  template<typename T, bool spsc>
  class LfQueue {
    static_assert(std::is_nothrow_move_constructible_v<T>);

  public:
    explicit LfQueue(std::size_t max_size) noexcept;

  private:
    std::vector<T> data_;
    std::atomic<std::size_t> push_ptr_ = 0;
    std::atomic<std::size_t> pop_ptr_ = 0;
  };

} // namespace algo
