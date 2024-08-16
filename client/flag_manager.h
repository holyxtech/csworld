#ifndef FLAG_MANAGER_H
#define FLAG_MANAGER_H

template <typename Flags>
class FlagManager {
public:
  void set_flag(Flags flag) { flags_ |= static_cast<std::uint32_t>(flag); }
  void unset_flag(Flags flag) { flags_ &= ~static_cast<std::uint32_t>(flag); }
  bool check_flag(Flags flag) const { return flags_ & static_cast<std::uint32_t>(flag); }

protected:
  std::uint32_t flags_;
};

#endif