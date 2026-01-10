TRACE_EVENT_NAME(EV_GST_START, "gst_telemetry_start")
TRACE_EVENT_NAME(EV_GST_STOP, "gst_telemetry_stop")
TRACE_EVENT_NAME(EV_GST_TRANSFORM_FRAME, "gst_telemetry_transform_frame_ip")
TRACE_EVENT_NAME(EV_GST_PREPARE_BUFFER, "gst_telemetry_transform_frame_ip prepare buffer")
TRACE_EVENT_NAME(EV_GST_PREPARE_COMPOSITION, "gst_telemetry_transform_frame_ip prepare composition")
TRACE_EVENT_NAME(EV_GST_BLEND_OVERLAY, "gst_telemetry_transform_frame_ip blend overlay (CPU mode)")
TRACE_EVENT_NAME(EV_GST_CLEANUP_RESOURCES, "gst_telemetry_transform_frame_ip cleanup resources")

TRACE_EVENT_NAME(EV_MANAGER_INIT, "manager::init")
TRACE_EVENT_NAME(EV_MANAGER_DEINIT, "manager::deinit")
TRACE_EVENT_NAME(EV_MANAGER_DRAW, "manager::draw")
TRACE_EVENT_NAME(EV_MANAGER_CLEAR_SURFACE, "manager::draw clear surface")

TRACE_EVENT_NAME(EV_TRACK_LOAD, "track::load")
TRACE_EVENT_NAME(EV_TRACK_LOAD_CUSTOM_DATA, "track::load_custom_data")

TRACE_EVENT_NAME(EV_LAYOUT_LOAD, "layout::load")
TRACE_EVENT_NAME(EV_LAYOUT_DRAW, "layout::draw")

TRACE_EVENT_NAME(EV_CHART_WIDGET_DRAW, "chart_widget::draw")
TRACE_EVENT_NAME(EV_CHART_WIDGET_UPDATE_LINE_CACHE, "chart_widget::draw update line cache")
TRACE_EVENT_NAME(EV_CHART_WIDGET_UPDATE_POINT_CACHE, "chart_widget::draw update point cache")
TRACE_EVENT_NAME(EV_CHART_WIDGET_DRAW_LINE_CACHE, "chart_widget::draw draw line from cache")
TRACE_EVENT_NAME(EV_CHART_WIDGET_DRAW_POINT_CACHE, "chart_widget::draw draw point from cache")

TRACE_EVENT_NAME(EV_CIRCLE_WIDGET_DRAW, "circle_widget::draw")
TRACE_EVENT_NAME(EV_CIRCLE_WIDGET_UPDATE_CACHE, "circle_widget::draw update cache")
TRACE_EVENT_NAME(EV_CIRCLE_WIDGET_DRAW_CACHE, "circle_widget::draw draw from cache")

TRACE_EVENT_NAME(EV_LINE_WIDGET_DRAW, "line_widget::draw")
TRACE_EVENT_NAME(EV_LINE_WIDGET_UPDATE_CACHE, "line_widget::draw update cache")
TRACE_EVENT_NAME(EV_LINE_WIDGET_DRAW_CACHE, "line_widget::draw draw from cache")

TRACE_EVENT_NAME(EV_RECTANGLE_WIDGET_DRAW, "rectangle_widget::draw")
TRACE_EVENT_NAME(EV_RECTANGLE_WIDGET_UPDATE_CACHE, "rectangle_widget::draw update cache")
TRACE_EVENT_NAME(EV_RECTANGLE_WIDGET_DRAW_CACHE, "rectangle_widget::draw draw from cache")

TRACE_EVENT_NAME(EV_STRING_WIDGET_DRAW, "string_widget::draw")
TRACE_EVENT_NAME(EV_STRING_WIDGET_UPDATE_CACHE, "string_widget::draw update cache")
TRACE_EVENT_NAME(EV_STRING_WIDGET_DRAW_CACHE, "string_widget::draw draw from cache")

