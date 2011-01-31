<?php
// $Id$

/**
 * A handler to provide a field that is constructed by the administrator using PHP.
 *
 * @ingroup views_field_handlers
 */
class views_php_handler_field extends views_handler_field {


  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['use_php_setup'] = array('default' => FALSE);
    $options['php_setup'] = array('default' => '');
    $options['php_value'] = array('default' => '');
    $options['php_output'] = array('default' => '');
    $options['use_php_click_sortable'] = array('default' => FALSE);
    $options['php_click_sortable'] = array('default' => FALSE);
    return $options;
  }

  /**
   * Implements views_handler#options_form().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form += views_php_form_element($this,
      array('use_php_setup', t('Use setup code'), t('If checked, you can provide PHP code to be run once right before execution of the view. This may be useful to define functions to be re-used in the value and/or output code.')),
      array('php_setup', t('Setup code'), t('Code to run right before execution of the view.'), false),
      array('$view', '$handler', '$static')
    );
    $form += views_php_form_element($this,
      FALSE,
      array('php_value', t('Value code'), t('Code to construct the value of this field.'), false),
      array('$view', '$handler', '$static', '$row', '$data')
    );
    $form += views_php_form_element($this,
      array('use_php_click_sortable', t('Enable click sort'), t('If checked, you can use PHP code to enable click sort on the field.')),
      array('php_click_sortable', t('Click sort code'), t('The comparison code must return an integer less than, equal to, or greater than zero if the first row should respectively appear before, stay where it was compared to, or appear after the second row.'), false),
      array(
        '$row1' => t('Data of row.'),
        '$row2' => t('Data of row to compare against.'),
      )
    );
    $form += views_php_form_element($this,
      FALSE,
      array('php_output', t('Output code'), t('Code to construct the output of this field.'), true),
      array('$view', '$handler', '$static', '$row', '$data')
    );
  }

  /**
   * Implements views_handler_field#query().
   *
   * @see views_php_views_pre_execute()
   */
  function query() {
    // Provide an field alias but don't actually alter the query.
    $this->field_alias = 'views_php_' . $this->position;
    // Inform views_php_views_pre_execute() to seize control over the query.
    $this->view->views_php = TRUE;
  }

  /**
   * Implements views_handler_field#click_sortable().
   */
  function click_sortable() {
    return !empty($this->options['php_click_sortable']);
  }

  /**
   * Implements views_handler_field#click_sort().
   *
   * @see self::php_post_execute()
   */
  function click_sort($order) {
    $this->php_click_sort_order = $order;
  }

  /**
   *
   * @see views_php_views_pre_execute()
   * @see self::php_post_execute()
   */
  function php_pre_execute() {
    // Ecexute static PHP code.
    if (!empty($this->options['php_setup'])) {
      $lamda_function = create_function('$view, $handler, &$static', $this->options['php_setup']);
      ob_start();
      $lamda_function($this->view, $this, &$this->php_static_variable);
      ob_end_clean();
    }
  }

  /**
   *
   * @see views_php_views_post_execute()
   */
  function php_post_execute() {
    // Ecexute value PHP code.
    if (!empty($this->options['php_value'])) {
      $lamda_function = create_function('$view, $handler, &$static, $row, $data', $this->options['php_value']);
      ob_start();
      foreach ($this->view->result as $i => &$row) {
        $normalized_row = new stdClass;
        foreach ($this->view->display_handler->get_handlers('field') as $field => $handler) {
          $normalized_row->$field = isset($row->{$handler->field_alias}) ? $row->{$handler->field_alias} : NULL;
        }
        $row->{$this->field_alias} = $lamda_function($this->view, $this, $this->php_static_variable, $normalized_row, $row);
      }
      ob_end_clean();
    }

    // If we're sorting, do the actual sorting then fix the results as per the pager info.
    if (!empty($this->options['php_click_sortable']) && !empty($this->php_click_sort_order)) {
      $lamda_function = create_function('$row1, $row2', $this->options['php_click_sortable']);
      ob_start();
      usort($this->view->result, $lamda_function);
      ob_end_clean();
    }
  }

  /**
   * Implements views_handler_field#pre_render().
   */
  function pre_render(&$values) {
    if (!empty($this->options['php_output'])) {
      $this->php_output_lamda_function = create_function('$view, &$static, $row, $data, $value', ' ?> ' . $this->options['php_output'] . ' <?php ');
    }
  }

  /**
   * Implements views_handler_field#render().
   */
  function render($values) {
    // Ecexute output PHP code.
    if (!empty($this->options['php_output']) && isset($this->php_output_lamda_function)) {
      $normalized_row = new stdClass;
      foreach ($this->view->display_handler->get_handlers('field') as $field => $handler) {
        $normalized_row->$field = isset($values->{$handler->field_alias}) ? $values->{$handler->field_alias} : NULL;
      }

      $lamda_function = $this->php_output_lamda_function;
      ob_start();
      $lamda_function($this->view, &$this->php_static_variable, $normalized_row, $values, isset($values->{$this->field_alias}) ? $values->{$this->field_alias} : NULL);
      $value = ob_get_clean();
    }
    else {
      $value = check_plain($values->{$this->field_alias});
    }
    return $value;
  }
}
