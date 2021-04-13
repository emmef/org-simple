//
// Created by michel on 11-04-21.
//

#include <org-simple/dsp/rate.h>
#include <type_traits>

template <typename T, typename R> struct IntegrationLogic {
  static_assert(!std::is_floating_point_v<T> || std::is_same_v<T, R>);
  static_assert(std::is_floating_point_v<T> || std::is_same_v<double, R>);

  template <typename X> static constexpr X float_maximum() {
    return (X)(-1.0) / log((T)1.0 - std::numeric_limits<T>::epsilon());
  }

  static constexpr T minimum = 0;
  static constexpr T maximum =
      std::is_floating_point_v<T>
          ? float_maximum<T>()
          : std::min((double)std::numeric_limits<T>::max(),
                     float_maximum<double>());

  static R unchecked_count_to_history_multiplier(T count) {
    return exp((R)(-1.0) / count);
  }
  static R unchecked_multiplier_to_multiplier(T multiplier) {
    return (R)1.0 - multiplier;
  }
  static R count_to_history_multiplier_bounded(T count) {
    if (count <= minimum) {
      return 0;
    }
    return unchecked_count_to_history_multiplier(std::min(count, maximum));
  }
  static T count_to_history_multiplier(T count) {
    if (count <= maximum) {
      return count_to_history_multiplier_bounded(count);
    }
    throw std::invalid_argument("org::simple::dsp::integration::sample_count_"
                                "range: count too large to represent.");
  }

  IntegrationLogic() : history_multiplier(0), input_multiplier(1) {}
  IntegrationLogic(T sample_count, bool check = true)
      : history_multiplier(
            check ? count_to_history_multiplier(sample_count)
                  : count_to_history_multiplier_bounded(sample_count)),
        input_multiplier(
            unchecked_multiplier_to_multiplier(history_multiplier)) {}

  template <typename S> S integrate(S history, S input) const {
    return history_multiplier * history + input_multiplier * input;
  }
  template <typename S> void integrated(S &history, S input) const {
    history *= history_multiplier;
    history += input_multiplier * input;
  }
  template <typename S> S get_integrated(S &history, S input) const {
    integrated(history, input);
    return history;
  }
  T get_sample_count() const { return (T)-1.0 / log(history_multiplier); }
  void set_sample_count_bounded(T sample_count) {
    history_multiplier = count_to_history_multiplier_bounded(sample_count);
    input_multiplier = unchecked_multiplier_to_multiplier(history_multiplier);
  }
  void set_sample_count(T sample_count) {
    history_multiplier = count_to_history_multiplier(sample_count);
    input_multiplier = unchecked_multiplier_to_multiplier(history_multiplier);
  }

private:
  R history_multiplier;
  R input_multiplier;
};

template <typename T, bool fp = std::is_floating_point_v<T>>
struct IntegrationBase;

template <typename T>
struct IntegrationBase<T, true> : public IntegrationLogic<T, T> {
  IntegrationBase() : IntegrationLogic<T, T>() {}
  IntegrationBase(T sample_count, bool check = true)
      : IntegrationLogic<T, T>(sample_count, check) {}
};

template <typename T>
struct IntegrationBase<T, false> : public IntegrationLogic<T, double> {
  IntegrationBase() : IntegrationLogic<T, double>() {}
  IntegrationBase(T sample_count, bool check = true)
      : IntegrationLogic<T, double>(sample_count, check) {}
};

typedef IntegrationBase<double> Integrator;
typedef IntegrationBase<long unsigned> IntegratorWhole;

// TODO Test coverage!
