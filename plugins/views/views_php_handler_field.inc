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
    $options['use_php_static'] = array('default' => FALSE);
    $options['php_static'] = array('default' => '');
    $options['php_value'] = array('default' => '');
    $options['php_output'] = array('default' => '');
    $options['php_click_sortable'] = array('default' => FALSE);
    return $options;
  }

  /**
   * Implements views_handler#option_definition().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['use_php_static'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use static PHP code'),
      '#description' => t('If checked, you can provide PHP code to be run once right before execution of the view. This may be useful to define functions to be re-used in the value and/or output code.'),
      '#default_value' => $this->options['use_php_static'] && !empty($this->options['php_static']),
    );
    $form['php_static'] = array(
      '#type' => 'textarea',
      '#title' => t('Static code'),
      '#default_value' => $this->options['php_static'],
      '#rows' => 5,
      '#description' =>
        t('Code to run right before execution of the view.') . ' <strong>' . t('Do not use &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong><br/>' .
        t('Available variables:') . '<br/>' .
        t('$static: can be used to store reusable data per row.')
      ,
      '#states' => array(
        'invisible' => array(
          'input[name="options[use_php_static]"]' => array('checked' => FALSE),
        ),
      ),
    );
    views_php_check_access($form['php_static']);
    $form['php_value'] = array(
      '#type' => 'textarea',
      '#title' => t('Value code'),
      '#default_value' => $this->options['php_value'],
      '#rows' => 5,
      '#description' =>
        t('Code to construct the value of this field.') . ' <strong>' . t('Do not use &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong><br/>' .
        t('Available variables:') . '<br/>' .
        t('$data: contains the retrieved record from the database (e.g. $data->nid).') . '<br/>' .
        t('$static: can be used to store reusable data per row.')
      ,
    );
    views_php_check_access($form['php_value']);
    $form['php_output'] = array(
      '#type' => 'textarea',
      '#title' => t('Output code'),
      '#default_value' => $this->options['php_output'],
      '#rows' => 5,
      '#description' =>
        t('Code to construct the output of this field.') . ' <strong>' . t('Use &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong><br/>' .
        t('Available variables:') . '<br/>' .
        t('$data: contains the retrieved record from the database (e.g. $data->nid).') . '<br/>' .
        t('$static: can be used to store reusable data per row.')
      ,
    );
    views_php_check_access($form['php_output']);
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
    if (!empty($this->options['php_static'])) {
      $lamda_function = create_function('$view, $handler, &$static', $this->options['php_static']);
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
      $lamda_function = create_function('$view, $handler, &$static, $data', $this->options['php_value']);
      ob_start();
      foreach ($this->view->result as $i => &$row) {
        $row->{$this->field_alias} = $lamda_function($this->view, $this, $this->php_static_variable, $row);
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
      $this->php_output_lamda_function = create_function('$view, &$static, $data, $value', ' ?> ' . $this->options['php_output'] . ' <?php ');
    }
  }

  /**
   * Implements views_handler_field#render().
   */
  function render($values) {
    // Ecexute output PHP code.
    if (!empty($this->options['php_output']) && isset($this->php_output_lamda_function)) {
      $lamda_function = $this->php_output_lamda_function;
      ob_start();
      $lamda_function($view, &$this->php_static_variable, $values, isset($values->{$this->field_alias}) ? $values->{$this->field_alias} : NULL);
      $value = ob_get_clean();
    }
    else {
      $value = check_plain($values->{$this->field_alias});
    }
    return $value;
  }
}
