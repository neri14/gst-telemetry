#include "expression.h"

namespace telemetry {
namespace overlay {

Expression::Expression(const std::string& expression_str,
                       std::shared_ptr<track::Track> track)
            : track_(track) {
    std::vector<std::string> variable_list;
    if (!exprtk::collect_variables(expression_str, variable_list)) {
        log.error("Failed to collect variables from expression: {}", expression_str);
        valid_expr_ = false;
        return;
    }

    exprtk::symbol_table<double> symbol_table;

    for (const auto& var_name : variable_list) {
        track::field_id_t field_id = track_->get_field_id(var_name);
        if (field_id == track::INVALID_FIELD) {
            log.warning("Variable '{}' not found in track fields.", var_name);
            valid_expr_ = false;
            return;
        }
        variables_[field_id] = 0.0;
        symbol_table.add_variable(var_name, variables_[field_id]);
    }

    expression_.register_symbol_table(symbol_table);

    exprtk::parser<double> parser;
    if (parser.compile(expression_str, expression_)) {
        log.info("Expression compiled successfully: {}", expression_str);
        valid_expr_ = true;
    } else {
        log.error("Failed to compile expression: {}", expression_str);
        for (std::size_t i = 0; i < parser.error_count(); ++i) {
            auto error = parser.get_error(i);
            log.error("Parser error {}: {} at position {}", 
                     i, error.diagnostic, error.token.position);
        }
        valid_expr_ = false;
    }
}

double Expression::evaluate(time::microseconds_t timestamp) {
    log.debug("Evaluating expression at timestamp {}", timestamp);

    if (!valid_expr_) {
        log.error("Invalid expression, cannot evaluate.");
        return 0.0;
    }

    bool needs_evaluation = std::isnan(value_); // needs evaluation if never evaluated

    bool invalid = false;
    for (auto& [field_id, var_ref] : variables_) {
        double new_value = track_->get(field_id, timestamp).as_double();
        if (var_ref != new_value) {
            var_ref = new_value;
            needs_evaluation = true;
        }
        if (std::isnan(var_ref)) {
            invalid = true;
        }
    }
    if (invalid) {
        log.warning("One or more variables are NaN at timestamp {}, expression evaluation skipped.", timestamp);
        return 0.0;
    }

    if (needs_evaluation) {
        value_ = expression_.value();
        log.debug("Expression evaluated to {}", value_);
    } else {
        log.debug("Using cached expression value {}", value_);
    }

    return value_;
}

bool Expression::is_valid() const {
    return valid_expr_;
}

} // namespace overlay
} // namespace telemetry
